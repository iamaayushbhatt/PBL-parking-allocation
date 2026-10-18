#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include "metrics/metrics.h"

static bool near(double a, double b)
{
    return std::fabs(a - b) < 1e-9;
}

int main()
{
    // ---- emission factors and baseline ----
    assert(near(Metrics::emissionFactorFor("Standard"), 0.122));
    assert(near(Metrics::emissionFactorFor("Accessible"), 0.122));
    assert(near(Metrics::emissionFactorFor("Electric"), 0.0));
    assert(near(Metrics::emissionFactorFor("Hovercraft"), 0.122)); // unknown -> Standard
    assert(near(Metrics::baselineDistance({10, 20, 60}), 30));
    assert(near(Metrics::baselineDistance({}), 0));

    Metrics m;

    // V1 standard: arrives 0, parks 5, exits 65; drove 100 m, baseline 160 m
    m.recordArrival("V1", 0);
    assert(m.recordParked("V1", 5, 100, 160, "Standard"));
    assert(m.recordExit("V1", 65));

    // V2 electric: arrives 10, parks 12, exits 42; drove 50, baseline 80
    m.recordArrival("V2", 10);
    assert(m.recordParked("V2", 12, 50, 80, "Electric"));
    assert(m.recordExit("V2", 42));

    // V3 accessible: arrives 20, parks 28, still parked; drove 40, baseline 30
    m.recordArrival("V3", 20);
    assert(m.recordParked("V3", 28, 40, 30, "Accessible"));

    // per-vehicle values
    assert(near(m.timeToPark("V1"), 5));
    assert(near(m.parkingDuration("V1"), 60));
    assert(near(m.emissions("V1"), 100 * 0.122));
    assert(near(m.emissionsSaved("V1"), 60 * 0.122));
    assert(near(m.emissions("V2"), 0));          // electric: no exhaust
    assert(near(m.emissionsSaved("V2"), 0));
    assert(near(m.emissionsSaved("V3"), -10 * 0.122)); // route was longer than baseline

    // not exited yet / unknown vehicle
    assert(near(m.parkingDuration("V3"), -1));
    assert(near(m.timeToPark("nobody"), -1));

    // counts and aggregates
    assert(m.arrivedCount() == 3);
    assert(m.parkedCount() == 3);
    assert(m.exitedCount() == 2);
    assert(near(m.averageTimeToPark(), (5.0 + 2.0 + 8.0) / 3));
    assert(near(m.averageParkingDuration(), (60.0 + 30.0) / 2));
    assert(near(m.totalEmissions(), (100 + 0 + 40) * 0.122));
    assert(near(m.totalEmissionsSaved(), (60 + 0 - 10) * 0.122));

    // invalid event order is rejected
    assert(!m.recordParked("nobody", 1, 1, 1, "Standard")); // never arrived
    assert(!m.recordParked("V1", 70, 1, 1, "Standard"));    // already parked
    assert(!m.recordExit("nobody", 1));                     // unknown vehicle
    m.recordArrival("V4", 50);
    assert(!m.recordExit("V4", 60));                        // exit before parking
    assert(!m.recordExit("V1", 99));                        // already exited

    // empty metrics: no division by zero
    Metrics empty;
    assert(near(empty.averageTimeToPark(), 0));
    assert(near(empty.averageParkingDuration(), 0));

    std::ostringstream out;
    m.printReport(out);
    assert(out.str().find("Vehicles parked") != std::string::npos);
    std::cout << out.str();

    std::cout << "All metrics tests passed\n";
    return 0;
}