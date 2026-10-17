#include <cassert>
#include <climits>
#include <iostream>
#include <stdexcept>
#include <string>

#include "JsonParser.h"
#include "layout_loader.h"

// true if loading this text throws an error
static bool fails(const std::string &text)
{
    try
    {
        loadLayoutFromString(text);
    }
    catch (const std::runtime_error &)
    {
        return true;
    }
    return false;
}

int main()
{
    // ---- JSON parser ----
    JsonValue v = parseJson(R"({"a": [1, 2.5, -3], "b": "hi\n", "c": true, "d": null})");
    assert(v.at("a").items.size() == 3);
    assert(v.at("a").items[2].asInt() == -3);
    assert(v.at("a").items[1].asDouble() == 2.5);
    assert(v.at("b").asString() == "hi\n");
    assert(v.at("c").asBool());
    assert(v.has("d") && !v.has("zzz"));

    bool threw = false;
    try { parseJson("{\"a\": }"); } catch (const std::runtime_error &) { threw = true; }
    assert(threw);
    threw = false;
    try { parseJson("[1, 2"); } catch (const std::runtime_error &) { threw = true; }
    assert(threw);
    threw = false;
    try { parseJson("{} extra"); } catch (const std::runtime_error &) { threw = true; }
    assert(threw);

    // ---- sample layout file ----
    Layout L = loadLayout("data/sample_layout.json");
    assert(L.name == "Sample 3-aisle layout");
    assert(L.graph->getVertices() == 6);
    assert(L.slots.size() == 5);
    assert(L.slots[3].ev && !L.slots[3].accessible);
    assert(L.slots[4].accessible);
    assert(L.entries.size() == 1 && L.entries[0] == 0);
    assert(L.exits[0] == 5);
    assert(L.graph->getCoordinates(5) == std::make_pair(15, 0));
    assert(L.graph->getWeight(0, 1) == 5);   // two-way lanes exist in both directions
    assert(L.graph->getWeight(1, 0) == 5);
    assert(L.graph->getWeight(0, 5) == INT_MAX); // no such lane

    // ---- bad layouts are rejected with an error ----
    const std::string good = R"({"nodes":[{"id":0},{"id":1}],
        "lanes":[{"from":0,"to":1,"distance":4}],
        "entries":[0],"exits":[1]})";
    assert(!fails(good));
    assert(fails(R"({"nodes":[],"lanes":[],"entries":[0],"exits":[0]})"));                          // no nodes
    assert(fails(R"({"nodes":[{"id":0},{"id":0}],"lanes":[],"entries":[0],"exits":[0]})"));         // duplicate node
    assert(fails(R"({"nodes":[{"id":0}],"lanes":[{"from":0,"to":7,"distance":4}],"entries":[0],"exits":[0]})")); // bad lane node
    assert(fails(R"({"nodes":[{"id":0},{"id":1}],"lanes":[{"from":0,"to":1,"distance":0}],"entries":[0],"exits":[1]})")); // zero distance
    assert(fails(R"({"nodes":[{"id":0},{"id":1}],"lanes":[],"slots":[{"id":1,"node":0,"size":"HUGE"}],"entries":[0],"exits":[1]})")); // bad size
    assert(fails(R"({"nodes":[{"id":0},{"id":1}],"lanes":[],"exits":[1]})"));                       // no entry gate
    assert(fails("not json at all"));

    // missing file
    threw = false;
    try { loadLayout("data/does_not_exist.json"); } catch (const std::runtime_error &) { threw = true; }
    assert(threw);

    std::cout << "All layout tests passed\n";
    return 0;
}
