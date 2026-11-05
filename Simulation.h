// Simulation: the event-driven parking simulation.
// Console mode:     Simulation sim(layout, Simulation::Mode::Demo); sim.run();
// Interactive mode: Simulation sim(layout, Simulation::Mode::Interactive);
//                   then call advanceTo / spawnVehicle / exitLongestParked / snapshot / takeLog.
#ifndef SIMULATION_H
#define SIMULATION_H

#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Event.h"
#include "EventQueue.h"
#include "SimView.h"
#include "intervaltree.h"
#include "metrics/metrics.h"
#include "models/Slot_State.h"
#include "models/Ticket_Metrics.h"
#include "models/VehicleHierarchy.h"
#include "scenario/RandomScenario.h"
#include "visualization/layoutLoader.h"

class Simulation
{
public:
    enum class Mode
    {
        Demo,        // the fixed 13-vehicle scenario
        Random,      // vehicleCount random vehicles, generated from seed
        Interactive  // starts empty; the caller adds vehicles while it runs
    };

    Simulation(const std::string &layoutPath, Mode mode = Mode::Demo,
               int vehicleCount = 0, unsigned seed = 1);

    // ---- console mode: run to the end, print every line and the final report
    void run();

    // ---- interactive mode
    std::string layoutName() const { return layout.name; }
    std::vector<NodeView> nodes() const;    // junctions with coordinates and gate flags, never change
    void advanceTo(double minute);          // process every event up to this minute
    std::string spawnVehicle(const std::string &category, const std::string &size); // arrives now; returns its id
    bool exitLongestParked();               // false if nobody is parked
    void setRandomTraffic(bool on, unsigned seed);
    SimSnapshot snapshot() const;
    std::vector<std::string> takeLog();     // log lines since the last call
    double time() const { return now; }

private:
    struct Transit
    {
        std::vector<int> route;
        int slotId;
        double start; // minute the vehicle started driving
    };

    struct Reservation
    {
        std::string holder;
        int low, high;
    };

    Layout layout;
    Scenario scenario;
    std::vector<std::pair<int, int>> allLanes; // every lane once, for the screen
    EventQueue queue;
    Metrics metrics;
    AdminMetricsManager admin;
    double now = 0;

    std::vector<std::unique_ptr<ParkingSlot>> slots;
    std::map<int, int> slotNode;                          // slotId -> graph node
    std::map<std::string, std::unique_ptr<Vehicle>> vehicles;
    std::map<std::string, DemoVehicle> demo;              // vehicleId -> its plan (stay etc.)
    std::map<std::string, Transit> inTransit;             // vehicles currently driving
    std::map<std::string, int> parkedAt;                  // vehicleId -> park time
    std::map<std::string, int> parkedNow;                 // vehicles parked right now -> slotId
    std::set<std::string> leaving;                        // exit requested, not processed yet
    std::map<int, std::unique_ptr<IntervalTree>> bookings;  // slotId -> reserved windows
    std::map<int, Reservation> reservations;              // slotId -> who holds it
    std::set<std::pair<int, int>> closed;                 // closed lanes (u < v)
    std::map<std::string, double> parkedBaseline, parkedDistance;
    int tickets = 0, turnedAway = 0, detours = 0;

    // log: handlers write into `out`; flushOut() turns it into lines
    std::ostringstream out;
    std::vector<std::string> logLines;

    // random traffic in interactive mode
    bool randomOn = false;
    unsigned randomSeed = 1;
    double randomUntil = 0;
    int randomBatch = 0, randomCounter = 0, userCounter = 0;
    std::set<std::string> trafficIds;

    ParkingSlot *slotById(int id);
    void setLane(int u, int v, bool isClosed);
    void changeLoad(const std::vector<int> &route, int delta);
    bool suitable(const ParkingSlot &s, const Vehicle &v, int from, int to);
    void stamp(double t);
    void flushOut();
    void registerVehicle(const DemoVehicle &d, double at);
    void refillRandom();
    void processNext();
    std::pair<double, double> positionOnRoute(const std::vector<int> &route, double distance) const;

    void onLane(const Event &e, bool block);
    void onArrival(const Event &e);
    void onParked(const Event &e);
    void onExit(const Event &e);
    void report();
};

#endif
