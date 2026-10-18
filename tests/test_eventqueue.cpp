#include <cassert>
#include <iostream>
#include "EventQueue.h"

int main() {
    EventQueue q;
    q.pushEvent({30.0, VEHICLE_EXIT, "V1"});
    q.pushEvent({10.0, VEHICLE_ARRIVAL, "V1"});
    q.pushEvent({20.0, LANE_BLOCKED, "", -1, 3});
    q.pushEvent({10.0, VEHICLE_ARRIVAL, "V2"});  // same time as vehicle 1

    assert(q.popNextEvent().vehicleId == "V1");  // 10.0, pushed first
    assert(q.popNextEvent().vehicleId == "V2");  // 10.0, pushed second
    assert(q.popNextEvent().type == LANE_BLOCKED);  // 20.0
    assert(q.popNextEvent().timestamp == 30.0);
    assert(q.empty());

    std::cout << "All tests passed\n";
    return 0;
}
