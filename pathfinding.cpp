#include "pathfinding.h"

#include <queue>
#include <vector>
#include <utility>
#include <limits>
#include <algorithm>
#include <cmath>

using namespace std;

// DIJKSTRA ALGORITHM - ARRAY BASED
vector<int> Pathfinding::dijkstra(
    Graph &graph,
    int start,
    int destination)
{
    int n = graph.getVertices();
    const int INF = numeric_limits<int>::max();

    if (start < 0 || start >= n ||
        destination < 0 || destination >= n)
        return {};

    vector<int> dis(n, INF);
    vector<int> visited(n, 0);
    vector<int> parent(n, -1);

    dis[start] = 0;

    for (int i = 0; i < n; i++)
    {
        // extractMin() logic
        int minimum = INF;
        int u = -1;

        for (int j = 0; j < n; j++)
        {
            if (visited[j] == 0 && dis[j] < minimum)
            {
                minimum = dis[j];
                u = j;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;

        if (u == destination)
            break;

        // Relax neighboring nodes
        for (auto edge : graph.getNeighbors(u))
        {
            int v = edge.first;
            int weight = edge.second;

            if (visited[v] == 0 &&
                weight >= 0 &&
                dis[u] <= INF - weight &&
                dis[u] + weight < dis[v])
            {
                dis[v] = dis[u] + weight;
                parent[v] = u;
            }
        }
    }

    // Destination unreachable
    if (dis[destination] == INF)
        return {};

    // Reconstruct shortest path
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
    // Handle empty and single-node paths
    if (path.size() < 2)
    {
        return 0;
    }

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