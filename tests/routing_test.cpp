
#include <iostream>
#include <vector>
#include "../graph.h"
#include "../pathfinding.h"

using namespace std;

void displayRoute(
    string name,
    Graph &graph,
    vector<int> path)
{
    cout << "\n"
         << name << ":\n";

    if (path.empty())
    {
        cout << "No route found\n";
        return;
    }

    cout << "Route: ";

    for (int i = 0; i < path.size(); i++)
    {
        cout << path[i];

        if (i < path.size() - 1)
            cout << " -> ";
    }

    int cost =
        Pathfinding::calculateDistance(graph, path);

    cout << "\nTotal cost: " << cost << "\n";
}

int main()
{
    // Create a graph with 5 nodes
    Graph parking(5);

    // Set coordinates for A* heuristic
    parking.setCoordinates(0, 0, 0);
    parking.setCoordinates(1, 2, 0);
    parking.setCoordinates(2, 0, 3);
    parking.setCoordinates(3, 4, 0);
    parking.setCoordinates(4, 4, 3);

    // Add lanes: source, destination, distance, capacity
    parking.addEdge(0, 1, 2, 10);
    parking.addEdge(1, 3, 2, 10);
    parking.addEdge(3, 4, 3, 10);

    parking.addEdge(0, 2, 3, 10);
    parking.addEdge(2, 4, 5, 10);

    int start = 0;
    int destination = 4;

    cout << "=== PARKING ROUTING TEST ===\n";

    // TEST 1: Normal traffic
    cout << "\nTEST 1: Normal traffic\n";

    displayRoute(
        "Dijkstra",
        parking,
        Pathfinding::dijkstra(
            parking, start, destination));

    displayRoute(
        "A*",
        parking,
        Pathfinding::aStar(
            parking, start, destination));

    // TEST 2: Congestion on lane 0 -> 1
    cout << "\nTEST 2: Congestion on lane 0 -> 1\n";

    parking.updateLaneOccupancy(0, 1, 10);

    displayRoute(
        "Dijkstra",
        parking,
        Pathfinding::dijkstra(
            parking, start, destination));

    displayRoute(
        "A*",
        parking,
        Pathfinding::aStar(
            parking, start, destination));

    // TEST 3: Close lane 2 -> 4
    cout << "\nTEST 3: Close lane 2 -> 4\n";

    parking.setLaneClosed(2, 4, true);

    displayRoute(
        "Dijkstra",
        parking,
        Pathfinding::dijkstra(
            parking, start, destination));

    displayRoute(
        "A*",
        parking,
        Pathfinding::aStar(
            parking, start, destination));

    // TEST 4: Invalid/unreachable destination
    cout << "\nTEST 4: Unreachable destination\n";

    parking.setLaneClosed(3, 4, true);

    displayRoute(
        "Dijkstra",
        parking,
        Pathfinding::dijkstra(
            parking, start, destination));

    displayRoute(
        "A*",
        parking,
        Pathfinding::aStar(
            parking, start, destination));

    return 0;
}
