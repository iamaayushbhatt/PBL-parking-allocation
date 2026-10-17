#include "layoutLoader.h"

#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

#include "JsonParser.h"

using namespace std;

static void check(bool ok, const string &msg)
{
    if (!ok)
        throw runtime_error("Layout error: " + msg);
}

static vector<int> readNodeList(const JsonValue &root, const string &key, int nodeCount)
{
    vector<int> result;
    if (!root.has(key))
        return result;
    for (const JsonValue &v : root.at(key).items)
    {
        int n = v.asInt();
        check(n >= 0 && n < nodeCount, "\"" + key + "\" refers to unknown node " + to_string(n));
        result.push_back(n);
    }
    return result;
}

Layout loadLayoutFromString(const string &jsonText)
{
    JsonValue root = parseJson(jsonText);
    check(root.isObject(), "the file must contain one JSON object");

    // ----- nodes -----
    const JsonValue &nodes = root.at("nodes");
    check(nodes.isArray() && !nodes.items.empty(), "\"nodes\" must be a non-empty list");
    int nodeCount = (int)nodes.items.size();

    Layout layout;
    layout.name = root.has("name") ? root.at("name").asString() : "unnamed layout";
    layout.graph = make_unique<Graph>(nodeCount);

    set<int> seenNodes;
    for (const JsonValue &n : nodes.items)
    {
        int id = n.at("id").asInt();
        check(id >= 0 && id < nodeCount, "node id " + to_string(id) + " must be between 0 and " + to_string(nodeCount - 1));
        check(seenNodes.insert(id).second, "node id " + to_string(id) + " is used twice");
        int x = n.has("x") ? n.at("x").asInt() : 0;
        int y = n.has("y") ? n.at("y").asInt() : 0;
        layout.graph->setCoordinates(id, x, y);
    }

    // ----- lanes -----
    for (const JsonValue &l : root.at("lanes").items)
    {
        int from = l.at("from").asInt();
        int to = l.at("to").asInt();
        int distance = l.at("distance").asInt();
        int capacity = l.has("capacity") ? l.at("capacity").asInt() : 10;
        bool twoWay = l.has("twoWay") ? l.at("twoWay").asBool() : false;

        check(from >= 0 && from < nodeCount && to >= 0 && to < nodeCount,
              "lane " + to_string(from) + "->" + to_string(to) + " uses an unknown node");
        check(from != to, "lane " + to_string(from) + "->" + to_string(to) + " starts and ends at the same node");
        check(distance > 0, "lane " + to_string(from) + "->" + to_string(to) + " needs a distance above 0");
        check(capacity > 0, "lane " + to_string(from) + "->" + to_string(to) + " needs a capacity above 0");

        layout.graph->addEdge(from, to, distance, capacity);
        if (twoWay)
            layout.graph->addEdge(to, from, distance, capacity);
    }

    // ----- slots -----
    set<int> seenSlots;
    if (root.has("slots"))
    {
        for (const JsonValue &sl : root.at("slots").items)
        {
            SlotSpec s;
            s.id = sl.at("id").asInt();
            s.node = sl.at("node").asInt();
            s.floor = sl.has("floor") ? sl.at("floor").asInt() : 0;
            s.size = sl.has("size") ? sl.at("size").asString() : "STANDARD";
            s.ev = sl.has("ev") ? sl.at("ev").asBool() : false;
            s.accessible = sl.has("accessible") ? sl.at("accessible").asBool() : false;

            check(s.node >= 0 && s.node < nodeCount, "slot " + to_string(s.id) + " is on unknown node " + to_string(s.node));
            check(seenSlots.insert(s.id).second, "slot id " + to_string(s.id) + " is used twice");
            check(s.size == "COMPACT" || s.size == "STANDARD" || s.size == "LARGE",
                  "slot " + to_string(s.id) + " has size \"" + s.size + "\" (use COMPACT, STANDARD or LARGE)");
            layout.slots.push_back(s);
        }
    }

    // ----- gates -----
    layout.entries = readNodeList(root, "entries", nodeCount);
    layout.exits = readNodeList(root, "exits", nodeCount);
    check(!layout.entries.empty(), "at least one entry gate is needed (\"entries\")");
    check(!layout.exits.empty(), "at least one exit gate is needed (\"exits\")");

    return layout;
}

Layout loadLayout(const string &path)
{
    ifstream in(path);
    if (!in)
        throw runtime_error("cannot open layout file: " + path);
    stringstream buffer;
    buffer << in.rdbuf();
    return loadLayoutFromString(buffer.str());
}