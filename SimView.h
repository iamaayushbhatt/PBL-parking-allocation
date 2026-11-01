// SimView.h -- everything the screen is allowed to see.
// The visualiser reads these structs and never touches the simulation's internals.
#pragma once
#include <string>
#include <vector>

struct NodeView    { int id; double x, y; bool isEntry, isExit; };   // gates are marked so the map can show them
struct LaneView    { int from, to; bool closed; int vehicles, capacity; };
struct SlotView    { int id, node; bool occupied, ev, accessible; std::string size, occupant; };
struct CarView     { std::string id, category, size;
                     bool parked;            // false = still driving to its slot
                     double x, y;            // position on the map right now
                     int slotId;             // slot it is heading to or parked in
                     std::vector<int> route; // junctions it is driving along; empty once parked
                   };
struct MetricsView { int arrived, parked, exited, turnedAway, rerouted;
                     double avgTimeToPark, avgDuration, totalCO2, co2Saved, revenue; };

struct SimSnapshot {
    double time;                  // simulation minute
    std::vector<LaneView> lanes;
    std::vector<SlotView> slots;
    std::vector<CarView> cars;
    MetricsView metrics;
};
