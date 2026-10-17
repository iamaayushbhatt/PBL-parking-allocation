#ifndef LAYOUT_LOADER_H
#define LAYOUT_LOADER_H

#include <memory>
#include <string>
#include <vector>

#include "../graph.h"

// One parking slot as described in the layout file.
// (The orchestrator turns these into ParkingSlot objects.)
struct SlotSpec
{
    int id;
    int node;         // graph node where the slot is
    int floor;
    std::string size; // "COMPACT", "STANDARD" or "LARGE"
    bool ev;
    bool accessible;
};

struct Layout
{
    std::string name;
    std::unique_ptr<Graph> graph; // lanes, distances, capacities, coordinates
    std::vector<SlotSpec> slots;
    std::vector<int> entries;     // entry gate nodes
    std::vector<int> exits;       // exit gate nodes
};

// Both throw std::runtime_error with a readable message if the file is wrong.
Layout loadLayoutFromString(const std::string &jsonText);
Layout loadLayout(const std::string &path);

#endif