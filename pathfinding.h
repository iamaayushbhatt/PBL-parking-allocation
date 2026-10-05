#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "Graph.h"
#include <vector>

class Pathfinding
{
public:
    // Dijkstra Algorithm
    static std::vector<int> dijkstra(
        Graph &graph,
        int start,
        int destination);

    // A* Algorithm
    static std::vector<int> aStar(
        Graph &graph,
        int start,
        int destination);

    // Calculate total distance of a path
    static int calculateDistance(
        Graph &graph,
        std::vector<int> path);

private:
    // Heuristic used by A*
    static int heuristic(
        Graph &graph,
        int current,
        int destination);
};

#endif