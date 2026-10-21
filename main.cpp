// Orchestrator: runs the whole simulation in plain text.
//   load layout -> queue events -> for each event: allocate slot, route,
//   update lanes, record metrics and tickets -> print report.
//
// Build (one command, from the repo root):
//   g++ -std=c++17 -I. main.cpp graph.cpp pathfinding.cpp intervaltree.cpp models/Slot_State.cpp models/VehicleHierarchy.cpp models/Ticket_Metrics.cpp metrics/metrics.cpp visualization/JsonParser.cpp visualization/layoutLoader.cpp -o parking_sim
// Run:
//   ./parking_sim layouts/DemoLayout.json

#include <climits>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Event.h"
#include "EventQueue.h"
#include "graph.h"
#include "intervaltree.h"
#include "pathfinding.h"
#include "metrics/metrics.h"
#include "models/Slot_State.h"
#include "models/Ticket_Metrics.h"
#include "models/VehicleHierarchy.h"
#include "visualization/layoutLoader.h"

using namespace std;

// ---------------------------------------------------------------- settings
static const double SPEED = 2.0; // distance units driven per minute

// ------------------------------------------------------ demo scenario data
struct DemoVehicle
{
    string id, plate, category, size;
    int arrival; // minutes
    int stay;    // minutes parked
};

static const vector<DemoVehicle> DEMO_VEHICLES = {
    {"V1", "UK07-1001", "Standard", "STANDARD", 0, 60},
    {"V2", "UK07-1002", "Electric", "STANDARD", 2, 45},
    {"V3", "UK07-1003", "Standard", "COMPACT", 3, 30},
    {"V4", "UK07-1004", "Accessible", "STANDARD", 5, 50},
    {"V5", "UK07-1005", "Standard", "STANDARD", 8, 40},
    {"V6", "UK07-1006", "Standard", "STANDARD", 12, 35}, // arrives while lane 1<->5 is closed
    {"V7", "UK07-1007", "Standard", "STANDARD", 14, 25},
    {"V8", "UK07-1008", "Electric", "STANDARD", 16, 30},
    {"V9", "UK07-1009", "Standard", "LARGE", 18, 30},    // slot 6 is booked by V11, so V9 goes elsewhere
    {"V10", "UK07-1010", "Standard", "STANDARD", 20, 40},
    {"V11", "UK07-1011", "Standard", "LARGE", 26, 30},   // holds the booking on slot 6
    {"V12", "UK07-1012", "Electric", "STANDARD", 28, 20},
    {"V13", "UK07-1013", "Accessible", "STANDARD", 30, 20}, // the only accessible slot is taken
};

// lanes the demo closes and reopens: laneId -> {from, to} (both directions)
static const vector<pair<int, int>> DEMO_LANES = {{1, 5}};

// ------------------------------------------------- placeholder fee function
// TODO: replace with Aman's fee calculator when it is pushed.
static double calcFee(int minutes, const string &size, bool ev)
{
    double perHour = (size == "LARGE") ? 50 : (size == "COMPACT") ? 20 : 30;
    if (ev)
        perHour += 10; // charging surcharge
    int hours = max(1, (int)ceil(minutes / 60.0));
    return perHour * hours;
}

// ----------------------------------------------------------------- state
struct Transit
{
    vector<int> route;
    int slotId;
};

struct Reservation
{
    string holder;
    int low, high;
};

class Simulation
{
    Layout layout;
    EventQueue queue;
    Metrics metrics;
    AdminMetricsManager admin;

    vector<unique_ptr<ParkingSlot>> slots;
    map<int, int> slotNode;                        // slotId -> graph node
    map<string, unique_ptr<Vehicle>> vehicles;
    map<string, const DemoVehicle *> demo;
    map<string, Transit> inTransit;
    map<string, int> parkedAt;                     // vehicleId -> park time
    map<string, int> parkedSlot;                   // vehicleId -> slotId
    map<int, unique_ptr<IntervalTree>> bookings;   // slotId -> reserved windows
    map<int, Reservation> reservations;            // slotId -> who holds it
    set<pair<int, int>> closed;                    // closed lanes (u < v)
    int tickets = 0, turnedAway = 0, detours = 0;

    ParkingSlot *slotById(int id)
    {
        for (auto &s : slots)
            if (s->getId() == id)
                return s.get();
        return nullptr;
    }

    void setLane(int u, int v, bool isClosed)
    {
        layout.graph->setLaneClosed(u, v, isClosed);
        layout.graph->setLaneClosed(v, u, isClosed);
        pair<int, int> key = {min(u, v), max(u, v)};
        if (isClosed)
            closed.insert(key);
        else
            closed.erase(key);
    }

    void changeLoad(const vector<int> &route, int delta)
    {
        for (size_t i = 0; i + 1 < route.size(); i++)
            layout.graph->updateLaneOccupancy(route[i], route[i + 1], delta);
    }

    // can this vehicle use this slot for the window [from, to)?
    bool suitable(const ParkingSlot &s, const Vehicle &v, int from, int to)
    {
        if (s.getState() != STATE_FREE)
            return false;
        if (s.getSizeRank() < v.getSizeRank())
            return false;
        if (s.checkEVZone() != v.requiresEV())
            return false; // EV zones are for EVs only, and EVs need them
        if (s.checkAccessibleZone() != v.requiresAccessible())
            return false;
        auto r = reservations.find(s.getId());
        bool holder = (r != reservations.end() && r->second.holder == v.getVehicleId());
        if (!holder && bookings.count(s.getId()) && bookings[s.getId()]->queryOverlap(from, to))
            return false; // someone else has booked that time
        return true;
    }

    void stamp(double t) { cout << "[t=" << setw(5) << fixed << setprecision(1) << t << "] "; }

public:
    explicit Simulation(const string &layoutPath) : layout(loadLayout(layoutPath))
    {
        for (const SlotSpec &sp : layout.slots)
        {
            slots.push_back(make_unique<ParkingSlot>(sp.id, sp.floor, sp.size, sp.ev, sp.accessible));
            slotNode[sp.id] = sp.node;
        }
        for (const DemoVehicle &d : DEMO_VEHICLES)
        {
            demo[d.id] = &d;
            if (d.category == "Electric")
                vehicles[d.id] = make_unique<ElectricVehicle>(d.id, d.plate, d.arrival, d.size);
            else if (d.category == "Accessible")
                vehicles[d.id] = make_unique<AccessibleVehicle>(d.id, d.plate, d.arrival, d.size);
            else
                vehicles[d.id] = make_unique<StandardVehicle>(d.id, d.plate, d.arrival, d.size);
            queue.pushEvent({(double)d.arrival, VEHICLE_ARRIVAL, d.id});
        }
        // lane closure and reopening
        queue.pushEvent({10.0, LANE_BLOCKED, "", -1, 0});
        queue.pushEvent({35.0, LANE_OPENED, "", -1, 0});

        // V11 has booked slot 6 from t=25 to t=70
        bookings[6] = make_unique<IntervalTree>();
        bookings[6]->tryReserve(25, 70);
        reservations[6] = {"V11", 25, 70};
    }

    void run()
    {
        cout << "Layout: " << layout.name << " (" << layout.graph->getVertices() << " nodes, "
             << slots.size() << " slots)\n\n";
        while (!queue.empty())
        {
            Event e = queue.popNextEvent();
            switch (e.type)
            {
            case VEHICLE_ARRIVAL: onArrival(e); break;
            case VEHICLE_PARKED:  onParked(e); break;
            case VEHICLE_EXIT:    onExit(e); break;
            case LANE_BLOCKED:    onLane(e, true); break;
            case LANE_OPENED:     onLane(e, false); break;
            }
        }
        report();
    }

private:
    void onLane(const Event &e, bool block)
    {
        pair<int, int> lane = DEMO_LANES[e.laneId];
        setLane(lane.first, lane.second, block);
        stamp(e.timestamp);
        cout << "LANE " << lane.first << "<->" << lane.second << (block ? " CLOSED" : " REOPENED") << "\n";
    }

    void onArrival(const Event &e)
    {
        const DemoVehicle &d = *demo[e.vehicleId];
        Vehicle &v = *vehicles[e.vehicleId];
        metrics.recordArrival(v.getVehicleId(), e.timestamp);
        stamp(e.timestamp);
        cout << v.getVehicleId() << " (" << v.getVehicleCategory() << ", " << v.getSize() << ") arrives\n";

        int entry = layout.entries[0];
        int from = (int)e.timestamp, to = from + d.stay;

        // find the best suitable slot (lowest congestion-weighted cost)
        ParkingSlot *best = nullptr;
        vector<int> bestRoute;
        int bestCost = INT_MAX;
        vector<double> candidateDistances; // for the baseline
        for (auto &sp : slots)
        {
            if (!suitable(*sp, v, from, to))
                continue;
            vector<int> route = Pathfinding::aStar(*layout.graph, entry, slotNode[sp->getId()]);
            if (route.empty())
                continue; // unreachable right now
            candidateDistances.push_back(layout.graph->pathDistance(route));
            int cost = Pathfinding::calculateDistance(*layout.graph, route);
            if (cost < bestCost)
            {
                bestCost = cost;
                best = sp.get();
                bestRoute = route;
            }
        }

        if (!best)
        {
            turnedAway++;
            stamp(e.timestamp);
            cout << "  -> no suitable slot, " << v.getVehicleId() << " turned away\n";
            return;
        }

        // did a closed lane force a detour? compare with the route if all lanes were open
        bool detour = false;
        if (!closed.empty())
        {
            set<pair<int, int>> saved = closed;
            for (auto &c : saved)
                setLane(c.first, c.second, false);
            vector<int> open = Pathfinding::aStar(*layout.graph, entry, slotNode[best->getId()]);
            for (auto &c : saved)
                setLane(c.first, c.second, true);
            detour = (open != bestRoute);
        }
        if (detour)
            detours++;

        double dist = layout.graph->pathDistance(bestRoute);
        double baseline = Metrics::baselineDistance(candidateDistances);
        stamp(e.timestamp);
        cout << "  -> slot " << best->getId() << ", route";
        for (size_t i = 0; i < bestRoute.size(); i++)
            cout << (i ? "->" : " ") << bestRoute[i];
        cout << " (" << dist << " units, baseline " << baseline << ")" << (detour ? "  [REROUTED around closed lane]" : "") << "\n";

        best->setState(STATE_RESERVED); // held for this vehicle while it drives
        changeLoad(bestRoute, +1);
        inTransit[v.getVehicleId()] = {bestRoute, best->getId()};
        parkedBaseline[v.getVehicleId()] = baseline;
        parkedDistance[v.getVehicleId()] = dist;
        queue.pushEvent({e.timestamp + dist / SPEED, VEHICLE_PARKED, v.getVehicleId(), best->getId()});
    }

    map<string, double> parkedBaseline, parkedDistance;

    void onParked(const Event &e)
    {
        Vehicle &v = *vehicles[e.vehicleId];
        Transit t = inTransit[e.vehicleId];
        inTransit.erase(e.vehicleId);
        changeLoad(t.route, -1);

        ParkingSlot *slot = slotById(t.slotId);
        slot->setState(STATE_FREE);
        slot->occupySlot(&v);
        metrics.recordParked(v.getVehicleId(), e.timestamp, parkedDistance[v.getVehicleId()],
                             parkedBaseline[v.getVehicleId()], v.getVehicleCategory());
        parkedAt[v.getVehicleId()] = (int)e.timestamp;
        parkedSlot[v.getVehicleId()] = t.slotId;

        vector<ParkingSlot *> all;
        for (auto &s : slots)
            all.push_back(s.get());
        stamp(e.timestamp);
        cout << v.getVehicleId() << " parked in slot " << t.slotId << "  (occupancy "
             << setprecision(0) << admin.calculateOccupancyRate(all) << "%)\n";
        queue.pushEvent({e.timestamp + demo[e.vehicleId]->stay, VEHICLE_EXIT, v.getVehicleId(), t.slotId});
    }

    void onExit(const Event &e)
    {
        Vehicle &v = *vehicles[e.vehicleId];
        ParkingSlot *slot = slotById(e.targetSlotId);
        slot->vacateSlot();
        metrics.recordExit(v.getVehicleId(), e.timestamp);

        int arrivalPark = parkedAt[v.getVehicleId()];
        int minutes = (int)e.timestamp - arrivalPark;
        double fee = calcFee(minutes, v.getSize(), v.requiresEV());
        admin.recordTicket(ParkingTicket("T" + to_string(++tickets), v.getVehicleId(), v.getLicensePlate(),
                                         slot->getId(), slot->getFloor(), arrivalPark, (int)e.timestamp, fee));
        stamp(e.timestamp);
        cout << v.getVehicleId() << " leaves slot " << slot->getId() << " after " << minutes
             << " min, fee " << fixed << setprecision(2) << fee << "\n";
    }

    void report()
    {
        vector<ParkingSlot *> all;
        for (auto &s : slots)
            all.push_back(s.get());
        cout << "\n";
        metrics.printReport(cout);
        cout << "Vehicles turned away   : " << turnedAway << "\n";
        cout << "Vehicles rerouted      : " << detours << "\n";
        admin.displayAdminDashboard(all);
    }
};

int main(int argc, char **argv)
{
    string path = (argc > 1) ? argv[1] : "layouts/DemoLayout.json";
    try
    {
        Simulation sim(path);
        sim.run();
    }
    catch (const exception &ex)
    {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
