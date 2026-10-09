#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "graph.h"
#include <vector>

class Pathfinding
{
public:
    // Dijkstra Algorithm
    static std::vector<int> dijkstra(
        Graph &graph,
        int start,
        int destination);

   
    static std::vector<int> aStar(
        Graph &graph,
        int start,
        int destination);

   
    static int calculateDistance(
        Graph &graph,
        std::vector<int> path);

private:
    
    static int heuristic(
        Graph &graph,
        int current,
        int destination);
};

#endif