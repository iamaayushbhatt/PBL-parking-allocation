#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <utility>

using namespace std;

struct Edge
{
    int to;           // node this lane leads to
    int distance;     // base length of the lane
    int capacity;     // how many vehicles fit comfortably
    int vehicleCount; // vehicles on the lane right now
    bool closed;      // true = lane is blocked
};

class Graph
{
private:
    int vertices;
    vector<vector<Edge>> adj;            // adj[u] = all lanes leaving node u
    vector<pair<int, int>> coordinates;  // x, y of every node
    Edge *findEdge(int u, int v);

public:
    Graph(int vertices);

    void addEdge(int source, int destination, int distance, int capacity = 10);
    void setCoordinates(int node, int x, int y);
    vector<pair<int, int>> getNeighbors(int node);
    pair<int, int> getCoordinates(int node);
    int getVertices();

    void updateLaneOccupancy(int u, int v, int delta);
    void setLaneClosed(int u, int v, bool isClosed);
    int getWeight(int u, int v);


    // base (physical) length: ignores traffic and closures
    int getDistance(int u, int v);             // INT_MAX if no such lane
    int pathDistance(const vector<int> &path); // 0 if fewer than 2 nodes
};

#endif