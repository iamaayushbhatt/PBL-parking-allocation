// Console test of the interactive side of Simulation (no screen needed).
// Build and run from the repo root (see README for the full command).
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

#include "Simulation.h"

static const CarView *findCar(const SimSnapshot &s, const std::string &id)
{
    for (const CarView &c : s.cars)
        if (c.id == id)
            return &c;
    return nullptr;
}

static const SlotView *findSlot(const SimSnapshot &s, int id)
{
    for (const SlotView &v : s.slots)
        if (v.id == id)
            return &v;
    return nullptr;
}

static bool near(double a, double b) { return std::fabs(a - b) < 0.001; }

int main()
{
    const std::string layout = "layouts/DemoLayout.json";

    // ---- stepping through the demo scenario
    {
        Simulation sim(layout, Simulation::Mode::Demo);
        assert(sim.nodes().size() == 8);
        assert(sim.nodes()[1].x == 5 && sim.nodes()[1].y == 0);
        assert(sim.nodes()[0].isEntry && !sim.nodes()[0].isExit);      // gate markers
        assert(sim.nodes()[4].isExit && !sim.nodes()[4].isEntry);
        assert(!sim.nodes()[2].isEntry && !sim.nodes()[2].isExit);
        assert(!sim.layoutName().empty());

        SimSnapshot s = sim.snapshot();
        assert(s.time == 0 && s.cars.empty() && s.slots.size() == 12);
        assert(s.lanes.size() == 9);

        // V1 arrives at t=0 and drives 0->1->5 at 2 units per minute
        sim.advanceTo(1.25);
        s = sim.snapshot();
        const CarView *v1 = findCar(s, "V1");
        assert(v1 != nullptr && !v1->parked);
        assert(near(v1->x, 2.5) && near(v1->y, 0));    // 2.5 units along lane 0->1
        assert(v1->slotId == 2);
        assert((v1->route == std::vector<int>{0, 1, 5}));              // driving: route is given

        sim.advanceTo(3.5);                             // 7 units driven: on lane 1->5
        s = sim.snapshot();
        v1 = findCar(s, "V1");
        assert(v1 != nullptr && near(v1->x, 5) && near(v1->y, 2));

        sim.advanceTo(5);                               // parked at slot 2
        s = sim.snapshot();
        v1 = findCar(s, "V1");
        assert(v1 != nullptr && v1->parked && v1->slotId == 2);
        assert(v1->route.empty());                                      // parked: no route
        assert(near(v1->x, 5) && near(v1->y, 5));       // node 5
        assert(findSlot(s, 2)->occupied && findSlot(s, 2)->occupant == "V1");
        assert(s.metrics.parked >= 1);

        // lane 1<->5 closes at t=10
        sim.advanceTo(10);
        s = sim.snapshot();
        bool lane15Closed = false;
        for (const LaneView &l : s.lanes)
            if (l.from == 1 && l.to == 5)
                lane15Closed = l.closed;
        assert(lane15Closed);

        std::vector<std::string> log = sim.takeLog();
        assert(!log.empty());
        assert(log[0].find("V1 (Standard, STANDARD) arrives") != std::string::npos);
        assert(sim.takeLog().empty());                  // the log is emptied once taken
    }

    // ---- interactive mode: spawn, park, leave
    {
        Simulation sim(layout, Simulation::Mode::Interactive);
        assert(sim.snapshot().cars.empty());
        assert(!sim.exitLongestParked());               // nobody to send away

        std::string id = sim.spawnVehicle("Standard", "STANDARD");
        assert(id == "U1");
        sim.advanceTo(0);
        SimSnapshot s = sim.snapshot();
        const CarView *c = findCar(s, id);
        assert(c != nullptr && !c->parked);

        sim.advanceTo(30);                              // plenty of time to park
        s = sim.snapshot();
        c = findCar(s, id);
        assert(c != nullptr && c->parked);
        assert(s.metrics.arrived == 1 && s.metrics.parked == 1 && s.metrics.exited == 0);

        std::string second = sim.spawnVehicle("Electric", "STANDARD");
        sim.advanceTo(31);
        sim.advanceTo(60);
        s = sim.snapshot();
        assert(findCar(s, second)->parked);

        assert(sim.exitLongestParked());                // U1 has been parked longest
        assert(sim.exitLongestParked());                // asking again picks U2, not U1 twice
        assert(!sim.exitLongestParked());               // nobody left to ask
        sim.advanceTo(61);
        s = sim.snapshot();
        assert(s.cars.empty());
        assert(s.metrics.exited == 2);
        assert(s.metrics.revenue > 0);
        for (const SlotView &sv : s.slots)
            assert(!sv.occupied);
    }

    // ---- random traffic on and off
    {
        Simulation sim(layout, Simulation::Mode::Interactive);
        sim.setRandomTraffic(true, 5);
        sim.advanceTo(100);
        int arrived = sim.snapshot().metrics.arrived;
        assert(arrived > 0);

        sim.setRandomTraffic(false, 0);
        sim.advanceTo(400);                             // cars scheduled earlier are ignored now
        assert(sim.snapshot().metrics.arrived == arrived);
    }

    std::cout << "All simulation tests passed\n";
    return 0;
}
