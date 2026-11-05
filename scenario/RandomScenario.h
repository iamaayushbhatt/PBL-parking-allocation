#ifndef RANDOM_SCENARIO_H
#define RANDOM_SCENARIO_H

#include <string>
#include <utility>
#include <vector>

#include "visualization/layoutLoader.h"

// One vehicle that will arrive during the simulation.
struct DemoVehicle
{
    std::string id, plate, category, size;
    int arrival; // minute it reaches the entry
    int stay;    // minutes it stays parked
};

// A lane that is closed at minute `start` and reopened at minute `end`.
// laneIndex is a position in Scenario::lanes.
struct LaneClosure
{
    int laneIndex;
    int start;
    int end;
};

// Everything the simulation needs to know about what will happen.
struct Scenario
{
    std::vector<DemoVehicle> vehicles;
    std::vector<std::pair<int, int>> lanes; // lanes that may be closed (u, v)
    std::vector<LaneClosure> closures;
};

// Every lane of the layout once; {1,5} stands for both 1->5 and 5->1.
std::vector<std::pair<int, int>> collectLanes(const Layout &layout);

// Builds a random scenario for any layout.
//   seed            same seed + same layout = same scenario (repeatable)
//   vehicleCount    how many vehicles arrive
//   durationMinutes arrivals are spread over minutes 0 .. durationMinutes
Scenario generateRandomScenario(const Layout &layout, unsigned seed,
                                int vehicleCount, int durationMinutes);

#endif
