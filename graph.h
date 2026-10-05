#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <utility>

using namespace std;

class Graph
{
private:
    int vertices;

    // adjacency list
    // pair = {destination, distance}
    vector<vector<pair<int, int>>> adj;

    // Coordinates of every node
    vector<pair<int, int>> coordinates;

public:
    Graph(int vertices);

    void addEdge(int source, int destination, int distance);

    void setCoordinates(int node, int x, int y);

    vector<pair<int, int>> getNeighbors(int node);

    pair<int, int> getCoordinates(int node);

    int getVertices();
};

#endif