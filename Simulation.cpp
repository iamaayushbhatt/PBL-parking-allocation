// Simulation: implementation. See Simulation.h for how to use it.
#include "Simulation.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <iomanip>
#include <iostream>

#include "graph.h"
#include "pathfinding.h"

using namespace std;

// ---------------------------------------------------------------- settings
static const double SPEED = 2.0; // distance units driven per minute

// ------------------------------------------------------ demo scenario data
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


// ----------------------------------------------------------- construction
Simulation::Simulation(const string &layoutPath, Mode mode, int vehicleCount, unsigned seed)
    : layout(loadLayout(layoutPath))
{
    allLanes = collectLanes(layout);

    if (mode == Mode::Random)
    {
        scenario = generateRandomScenario(layout, seed, vehicleCount, 180);
    }
    else if (mode == Mode::Demo)
    {
        scenario.vehicles = DEMO_VEHICLES;
        scenario.lanes = DEMO_LANES;
        scenario.closures = {{0, 10, 35}};
    }
    else
    {
        scenario.lanes = allLanes; // nothing is scheduled yet
    }

    for (const SlotSpec &sp : layout.slots)
    {
        slots.push_back(make_unique<ParkingSlot>(sp.id, sp.floor, sp.size, sp.ev, sp.accessible));
        slotNode[sp.id] = sp.node;
    }
    for (const DemoVehicle &d : scenario.vehicles)
        registerVehicle(d, d.arrival);

    // lane closures and reopenings
    for (const LaneClosure &c : scenario.closures)
    {
        queue.pushEvent({(double)c.start, LANE_BLOCKED, "", -1, c.laneIndex});
        queue.pushEvent({(double)c.end, LANE_OPENED, "", -1, c.laneIndex});
    }

    if (mode == Mode::Demo)
    {
        // V11 has booked slot 6 from t=25 to t=70
        bookings[6] = make_unique<IntervalTree>();
        bookings[6]->tryReserve(25, 70);
        reservations[6] = {"V11", 25, 70};
    }
}

// make the vehicle object and schedule its arrival at minute `at`
void Simulation::registerVehicle(const DemoVehicle &d, double at)
{
    demo[d.id] = d;
    if (d.category == "Electric")
        vehicles[d.id] = make_unique<ElectricVehicle>(d.id, d.plate, (int)at, d.size);
    else if (d.category == "Accessible")
        vehicles[d.id] = make_unique<AccessibleVehicle>(d.id, d.plate, (int)at, d.size);
    else
        vehicles[d.id] = make_unique<StandardVehicle>(d.id, d.plate, (int)at, d.size);
    queue.pushEvent({at, VEHICLE_ARRIVAL, d.id});
}

// ------------------------------------------------------------ running it
void Simulation::run()
{
    cout << "Layout: " << layout.name << " (" << layout.graph->getVertices() << " nodes, "
         << slots.size() << " slots)\n\n";
    while (!queue.empty())
    {
        processNext();
        for (const string &line : takeLog())
            cout << line << "\n";
    }
    report();
}

void Simulation::processNext()
{
    Event e = queue.popNextEvent();
    now = max(now, e.timestamp);
    switch (e.type)
    {
    case VEHICLE_ARRIVAL: onArrival(e); break;
    case VEHICLE_PARKED:  onParked(e); break;
    case VEHICLE_EXIT:    onExit(e); break;
    case LANE_BLOCKED:    onLane(e, true); break;
    case LANE_OPENED:     onLane(e, false); break;
    }
    flushOut();
}

void Simulation::advanceTo(double minute)
{
    refillRandom();
    while (!queue.empty() && queue.peek().timestamp <= minute)
        processNext();
    if (minute > now)
        now = minute;
}

// ---------------------------------------------------------------- the log
void Simulation::stamp(double t) { out << "[t=" << setw(5) << fixed << setprecision(1) << t << "] "; }

// split what the handlers wrote into whole lines
void Simulation::flushOut()
{
    string text = out.str();
    out.str("");
    size_t start = 0;
    while (start < text.size())
    {
        size_t end = text.find('\n', start);
        if (end == string::npos)
            end = text.size();
        logLines.push_back(text.substr(start, end - start));
        start = end + 1;
    }
}

vector<string> Simulation::takeLog()
{
    vector<string> lines;
    lines.swap(logLines);
    return lines;
}

// ------------------------------------------------------ interactive controls
string Simulation::spawnVehicle(const string &category, const string &size)
{
    DemoVehicle d;
    d.id = "U" + to_string(++userCounter);
    d.plate = "UK07-" + to_string(9000 + userCounter);
    d.category = category;
    d.size = size;
    d.arrival = (int)now;
    d.stay = 100000; // stays until the user asks it to leave
    registerVehicle(d, now);
    return d.id;
}

bool Simulation::exitLongestParked()
{
    string best;
    int bestTime = INT_MAX;
    for (const auto &entry : parkedNow)
    {
        if (leaving.count(entry.first))
            continue;
        int t = parkedAt.at(entry.first);
        if (t < bestTime)
        {
            bestTime = t;
            best = entry.first;
        }
    }
    if (best.empty())
        return false;
    leaving.insert(best);
    queue.pushEvent({now, VEHICLE_EXIT, best, parkedNow.at(best)});
    return true;
}

void Simulation::setRandomTraffic(bool on, unsigned seed)
{
    randomOn = on;
    if (on)
    {
        randomSeed = seed;
        randomUntil = now;
        scenario.lanes = allLanes; // closures below refer to this list
        refillRandom();
    }
}

// keep random cars scheduled at least 30 minutes ahead
void Simulation::refillRandom()
{
    while (randomOn && randomUntil < now + 30)
    {
        Scenario batch = generateRandomScenario(layout, randomSeed + randomBatch++, 30, 120);
        for (DemoVehicle d : batch.vehicles)
        {
            d.id = "R" + to_string(++randomCounter);
            d.arrival += (int)randomUntil;
            trafficIds.insert(d.id);
            registerVehicle(d, d.arrival);
        }
        for (const LaneClosure &c : batch.closures)
        {
            queue.pushEvent({randomUntil + c.start, LANE_BLOCKED, "", -1, c.laneIndex});
            queue.pushEvent({randomUntil + c.end, LANE_OPENED, "", -1, c.laneIndex});
        }
        randomUntil += 120;
    }
}

// ------------------------------------------------------------- the screen's view
vector<NodeView> Simulation::nodes() const
{
    vector<NodeView> result;
    for (int i = 0; i < layout.graph->getVertices(); i++)
    {
        pair<int, int> c = layout.graph->getCoordinates(i);
        bool isEntry = find(layout.entries.begin(), layout.entries.end(), i) != layout.entries.end();
        bool isExit = find(layout.exits.begin(), layout.exits.end(), i) != layout.exits.end();
        result.push_back({i, (double)c.first, (double)c.second, isEntry, isExit});
    }
    return result;
}

// where is a vehicle that has driven `distance` units along `route`?
pair<double, double> Simulation::positionOnRoute(const vector<int> &route, double distance) const
{
    Graph &g = *layout.graph;
    if (route.empty())
        return {0, 0};
    for (size_t i = 0; i + 1 < route.size(); i++)
    {
        int len = g.getDistance(route[i], route[i + 1]);
        pair<int, int> a = g.getCoordinates(route[i]);
        pair<int, int> b = g.getCoordinates(route[i + 1]);
        if (len == INT_MAX || len <= 0 || distance <= len)
        {
            double f = (len == INT_MAX || len <= 0) ? 1.0 : max(0.0, distance) / len;
            return {a.first + (b.first - a.first) * f, a.second + (b.second - a.second) * f};
        }
        distance -= len;
    }
    pair<int, int> last = g.getCoordinates(route.back());
    return {(double)last.first, (double)last.second};
}

SimSnapshot Simulation::snapshot() const
{
    SimSnapshot s;
    s.time = now;
    Graph &g = *layout.graph;

    for (const auto &lane : allLanes)
    {
        int u = lane.first, v = lane.second;
        LaneView l;
        l.from = u;
        l.to = v;
        l.closed = g.isClosed(u, v) || g.isClosed(v, u);
        l.vehicles = g.getVehicleCount(u, v) + g.getVehicleCount(v, u);
        l.capacity = max(g.getCapacity(u, v), g.getCapacity(v, u));
        s.lanes.push_back(l);
    }

    for (const auto &sp : slots)
    {
        SlotView sv;
        sv.id = sp->getId();
        sv.node = slotNode.at(sv.id);
        sv.occupied = sp->getState() != STATE_FREE;
        sv.ev = sp->checkEVZone();
        sv.accessible = sp->checkAccessibleZone();
        sv.size = sp->getSlotSize();
        Vehicle *occupant = sp->getOccupyingVehicle();
        sv.occupant = occupant ? occupant->getVehicleId() : "";
        s.slots.push_back(sv);
    }

    for (const auto &entry : inTransit) // still driving
    {
        const Transit &t = entry.second;
        const Vehicle &v = *vehicles.at(entry.first);
        pair<double, double> p = positionOnRoute(t.route, (now - t.start) * SPEED);
        s.cars.push_back({v.getVehicleId(), v.getVehicleCategory(), v.getSize(), false, p.first, p.second, t.slotId, t.route});
    }
    for (const auto &entry : parkedNow) // parked
    {
        const Vehicle &v = *vehicles.at(entry.first);
        pair<int, int> c = g.getCoordinates(slotNode.at(entry.second));
        s.cars.push_back({v.getVehicleId(), v.getVehicleCategory(), v.getSize(), true, (double)c.first, (double)c.second, entry.second, {}});
    }

    MetricsView &m = s.metrics;
    m.arrived = metrics.arrivedCount();
    m.parked = metrics.parkedCount();
    m.exited = metrics.exitedCount();
    m.turnedAway = turnedAway;
    m.rerouted = detours;
    m.avgTimeToPark = metrics.averageTimeToPark();
    m.avgDuration = metrics.averageParkingDuration();
    m.totalCO2 = metrics.totalEmissions();
    m.co2Saved = metrics.totalEmissionsSaved();
    m.revenue = admin.getTotalRevenue();
    return s;
}

// ---------------------------------------------------------------- helpers
ParkingSlot *Simulation::slotById(int id)
{
    for (auto &s : slots)
        if (s->getId() == id)
            return s.get();
    return nullptr;
}

void Simulation::setLane(int u, int v, bool isClosed)
{
    layout.graph->setLaneClosed(u, v, isClosed);
    layout.graph->setLaneClosed(v, u, isClosed);
    pair<int, int> key = {min(u, v), max(u, v)};
    if (isClosed)
        closed.insert(key);
    else
        closed.erase(key);
}

void Simulation::changeLoad(const vector<int> &route, int delta)
{
    for (size_t i = 0; i + 1 < route.size(); i++)
        layout.graph->updateLaneOccupancy(route[i], route[i + 1], delta);
}

// can this vehicle use this slot for the window [from, to)?
bool Simulation::suitable(const ParkingSlot &s, const Vehicle &v, int from, int to)
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

// ------------------------------------------------------------ event handlers
void Simulation::onLane(const Event &e, bool block)
{
    pair<int, int> lane = scenario.lanes[e.laneId];
    setLane(lane.first, lane.second, block);
    stamp(e.timestamp);
    out << "LANE " << lane.first << "<->" << lane.second << (block ? " CLOSED" : " REOPENED") << "\n";
}

void Simulation::onArrival(const Event &e)
{
    if (trafficIds.count(e.vehicleId) && !randomOn)
        return; // random traffic was switched off before this car arrived

    const DemoVehicle &d = demo.at(e.vehicleId);
    Vehicle &v = *vehicles[e.vehicleId];
    metrics.recordArrival(v.getVehicleId(), e.timestamp);
    stamp(e.timestamp);
    out << v.getVehicleId() << " (" << v.getVehicleCategory() << ", " << v.getSize() << ") arrives\n";

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
        out << "  -> no suitable slot, " << v.getVehicleId() << " turned away\n";
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
    out << "  -> slot " << best->getId() << ", route";
    for (size_t i = 0; i < bestRoute.size(); i++)
        out << (i ? "->" : " ") << bestRoute[i];
    out << " (" << dist << " units, baseline " << baseline << ")" << (detour ? "  [REROUTED around closed lane]" : "") << "\n";

    best->setState(STATE_RESERVED); // held for this vehicle while it drives
    changeLoad(bestRoute, +1);
    inTransit[v.getVehicleId()] = {bestRoute, best->getId(), e.timestamp};
    parkedBaseline[v.getVehicleId()] = baseline;
    parkedDistance[v.getVehicleId()] = dist;
    queue.pushEvent({e.timestamp + dist / SPEED, VEHICLE_PARKED, v.getVehicleId(), best->getId()});
}

void Simulation::onParked(const Event &e)
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
    parkedNow[v.getVehicleId()] = t.slotId;

    vector<ParkingSlot *> all;
    for (auto &s : slots)
        all.push_back(s.get());
    stamp(e.timestamp);
    out << v.getVehicleId() << " parked in slot " << t.slotId << "  (occupancy "
        << setprecision(0) << admin.calculateOccupancyRate(all) << "%)\n";
    queue.pushEvent({e.timestamp + demo.at(e.vehicleId).stay, VEHICLE_EXIT, v.getVehicleId(), t.slotId});
}

void Simulation::onExit(const Event &e)
{
    if (!parkedNow.count(e.vehicleId))
        return; // already left (its planned exit came after a forced one)
    parkedNow.erase(e.vehicleId);
    leaving.erase(e.vehicleId);

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
    out << v.getVehicleId() << " leaves slot " << slot->getId() << " after " << minutes
        << " min, fee " << fixed << setprecision(2) << fee << "\n";
}

void Simulation::report()
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
