#include <cassert>
#include <climits>
#include <iostream>
#include "graph.h"

int main()
{
    Graph g(3);
    g.addEdge(0, 1, 10, 5);
    g.addEdge(1, 2, 10, 5);

    assert(g.getWeight(0, 1) == 10);   // empty lane = base distance

    g.updateLaneOccupancy(0, 1, 5);    // full lane
    assert(g.getWeight(0, 1) == 30);   // 10 * (1 + 2 * 5/5)

    g.updateLaneOccupancy(0, 1, -4);   // 1 vehicle left
    assert(g.getWeight(0, 1) == 14);   // 10 * (1 + 2 * 1/5)

    g.updateLaneOccupancy(0, 1, -100); // can't go below zero
    assert(g.getWeight(0, 1) == 10);

    assert(g.getWeight(1, 0) == INT_MAX); // lanes are one-way

    g.setLaneClosed(0, 1, true);
    assert(g.getNeighbors(0).empty());
    assert(g.getWeight(0, 1) == INT_MAX);

    g.setLaneClosed(0, 1, false);
    assert(g.getNeighbors(0).size() == 1);
    assert(g.getNeighbors(0)[0].first == 1);

    g.setCoordinates(2, 7, 9);
    assert(g.getCoordinates(2).first == 7);
    assert(g.getCoordinates(2).second == 9);

    g.addEdge(2, 0, 4);                // old 3-argument call
    assert(g.getWeight(2, 0) == 4);

    std::cout << "All graph tests passed\n";
    return 0;
}