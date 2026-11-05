#include "scenario/RandomScenario.h"

#include <algorithm>
#include <random>
#include <set>

using namespace std;

// list every lane of the layout once (the pair {1,5} stands for 1->5 and 5->1)
vector<pair<int, int>> collectLanes(const Layout &layout)
{
    set<pair<int, int>> found;
    int n = layout.graph->getVertices();
    for (int u = 0; u < n; u++)
        for (const auto &edge : layout.graph->getNeighbors(u))
        {
            int v = edge.first;
            found.insert({min(u, v), max(u, v)});
        }
    return vector<pair<int, int>>(found.begin(), found.end());
}

// pick an entry from `choices` according to `weights` (weights add up to 100)
static string pick(mt19937 &rng, const vector<string> &choices, const vector<int> &weights)
{
    int roll = uniform_int_distribution<int>(1, 100)(rng);
    int total = 0;
    for (size_t i = 0; i < choices.size(); i++)
    {
        total += weights[i];
        if (roll <= total)
            return choices[i];
    }
    return choices.back();
}

Scenario generateRandomScenario(const Layout &layout, unsigned seed,
                                int vehicleCount, int durationMinutes)
{
    mt19937 rng(seed);
    Scenario s;

    if (durationMinutes < 1)
        durationMinutes = 1;

    // ---- vehicles
    for (int i = 1; i <= vehicleCount; i++)
    {
        DemoVehicle d;
        d.id = "R" + to_string(i);
        d.plate = "UK07-" + to_string(uniform_int_distribution<int>(1000, 9999)(rng));
        d.category = pick(rng, {"Standard", "Electric", "Accessible"}, {70, 20, 10});
        d.size = pick(rng, {"COMPACT", "STANDARD", "LARGE"}, {20, 60, 20});
        d.arrival = uniform_int_distribution<int>(0, durationMinutes)(rng);
        d.stay = uniform_int_distribution<int>(15, 90)(rng);
        s.vehicles.push_back(d);
    }

    // earliest arrival first, so R1 is the first to arrive
    sort(s.vehicles.begin(), s.vehicles.end(),
         [](const DemoVehicle &a, const DemoVehicle &b) { return a.arrival < b.arrival; });
    for (size_t i = 0; i < s.vehicles.size(); i++)
        s.vehicles[i].id = "R" + to_string(i + 1);

    // ---- lane closures: about one per 8 vehicles, each on a different lane
    s.lanes = collectLanes(layout);
    vector<int> order;
    for (size_t i = 0; i < s.lanes.size(); i++)
        order.push_back((int)i);
    shuffle(order.begin(), order.end(), rng);

    int closures = min((int)s.lanes.size(), max(1, vehicleCount / 8));
    if (s.lanes.empty())
        closures = 0;
    for (int k = 0; k < closures; k++)
    {
        int start = uniform_int_distribution<int>(0, durationMinutes)(rng);
        int length = uniform_int_distribution<int>(10, 30)(rng);
        s.closures.push_back({order[k], start, start + length});
    }
    return s;
}
