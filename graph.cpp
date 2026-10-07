#include "graph.h"
#include <cmath>
#include <climits>

// weight = distance * (1 + ALPHA * vehicleCount / capacity)
static const double ALPHA = 2.0;

static int weightOf(const Edge &e)
{
    double load = 0.0;
    if (e.capacity > 0)
        load = (double)e.vehicleCount / e.capacity;
    return (int)ceil(e.distance * (1.0 + ALPHA * load));
}

Graph::Graph(int v) : vertices(v), adj(v), coordinates(v, {0, 0}) {}

Edge *Graph::findEdge(int u, int v)
{
    for (Edge &e : adj[u])
        if (e.to == v)
            return &e;
    return nullptr;
}

void Graph::addEdge(int source, int destination, int distance, int capacity)
{
    adj[source].push_back(Edge{destination, distance, capacity, 0, false});
}

void Graph::setCoordinates(int node, int x, int y)
{
    coordinates[node] = {x, y};
}

pair<int, int> Graph::getCoordinates(int node)
{
    return coordinates[node];
}

int Graph::getVertices()
{
    return vertices;
}

vector<pair<int, int>> Graph::getNeighbors(int node)
{
    vector<pair<int, int>> result;
    for (const Edge &e : adj[node])
        if (!e.closed)
            result.push_back({e.to, weightOf(e)});
    return result;
}

void Graph::updateLaneOccupancy(int u, int v, int delta)
{
    Edge *e = findEdge(u, v);
    if (e == nullptr)
        return;
    e->vehicleCount += delta;
    if (e->vehicleCount < 0)
        e->vehicleCount = 0;
}

void Graph::setLaneClosed(int u, int v, bool isClosed)
{
    Edge *e = findEdge(u, v);
    if (e != nullptr)
        e->closed = isClosed;
}

int Graph::getWeight(int u, int v)
{
    Edge *e = findEdge(u, v);
    if (e == nullptr || e->closed)
        return INT_MAX;
    return weightOf(*e);
}