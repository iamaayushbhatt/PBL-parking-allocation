#include "pathfinding.h"

#include <queue>
#include <vector>
#include <utility>
#include <limits>
#include <algorithm>
#include <cmath>

using namespace std;


// DIJKSTRA ALGORITHM
vector<int> Pathfinding::dijkstra(
    Graph &graph,
    int start,
    int destination)
{
    int n = graph.getVertices();

    vector<int> distance(
        n,
        numeric_limits<int>::max());

    vector<int> parent(n, -1);

    priority_queue<
        pair<int, int>,
        vector<pair<int, int>>,
        greater<pair<int, int>>>
        pq;

    distance[start] = 0;

    pq.push({0, start});

    while (!pq.empty())
    {
        int currentDistance = pq.top().first;
        int current = pq.top().second;

        pq.pop();

        // Ignore old information in priority queue
        if (currentDistance > distance[current])
            continue;

        // Destination reached
        if (current == destination)
            break;

        // Check all neighboring nodes
        for (auto edge : graph.getNeighbors(current))
        {
            int next = edge.first;
            int weight = edge.second;

            int newDistance =
                currentDistance + weight;

            if (newDistance < distance[next])
            {
                distance[next] = newDistance;
                parent[next] = current;

                pq.push({newDistance,
                         next});
            }
        }
    }

    // No route found
    if (distance[destination] ==
        numeric_limits<int>::max())
    {
        return {};
    }

    // Reconstruct path
    vector<int> path;

    int current = destination;

    while (current != -1)
    {
        path.push_back(current);
        current = parent[current];
    }

    reverse(path.begin(), path.end());

    return path;
}


// HEURISTIC FOR A*
int Pathfinding::heuristic(
    Graph &graph,
    int current,
    int destination)
{
    pair<int, int> currentPoint =
        graph.getCoordinates(current);

    pair<int, int> destinationPoint =
        graph.getCoordinates(destination);

    int xDifference =
        abs(
            currentPoint.first -
            destinationPoint.first);

    int yDifference =
        abs(
            currentPoint.second -
            destinationPoint.second);

    // Manhattan distance
    return xDifference + yDifference;
}

// A* ALGORITHM
vector<int> Pathfinding::aStar(
    Graph &graph,
    int start,
    int destination)
{
    int n = graph.getVertices();

    vector<int> distance(
        n,
        numeric_limits<int>::max());

    vector<int> parent(n, -1);

    priority_queue<
        pair<int, int>,
        vector<pair<int, int>>,
        greater<pair<int, int>>>
        pq;

    distance[start] = 0;

    int initialPriority =
        heuristic(
            graph,
            start,
            destination);

    pq.push({initialPriority,
             start});

    while (!pq.empty())
    {
        int current = pq.top().second;

        pq.pop();

        if (current == destination)
            break;

        for (auto edge :
             graph.getNeighbors(current))
        {
            int next = edge.first;
            int weight = edge.second;

            int newDistance =
                distance[current] + weight;

            if (newDistance < distance[next])
            {
                distance[next] = newDistance;

                parent[next] = current;

                int priority =
                    newDistance +
                    heuristic(
                        graph,
                        next,
                        destination);

                pq.push({priority,
                         next});
            }
        }
    }

    // No route found
    if (distance[destination] ==
        numeric_limits<int>::max())
    {
        return {};
    }

    // Reconstruct path
    vector<int> path;

    int current = destination;

    while (current != -1)
    {
        path.push_back(current);
        current = parent[current];
    }

    reverse(path.begin(), path.end());

    return path;
}

// CALCULATE TOTAL DISTANCE
int Pathfinding::calculateDistance(
    Graph &graph,
    vector<int> path)
{
    int totalDistance = 0;

    for (int i = 0;
         i < path.size() - 1;
         i++)
    {
        int current = path[i];
        int next = path[i + 1];

        int weight =
            graph.getWeight(
                current,
                next);

        if (weight == numeric_limits<int>::max())
        {
            return numeric_limits<int>::max();
        }

        totalDistance += weight;
    }

    return totalDistance;
}