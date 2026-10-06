#pragma once

enum EventType { VEHICLE_ARRIVAL, VEHICLE_EXIT, LANE_BLOCKED, LANE_OPENED };

struct Event {
    double timestamp;
    EventType type;
    int vehicleId = -1;
    int targetSlotId = -1;
    int laneId = -1;
    long long seq = 0;   // set by the queue, not by the caller

    // Min-heap ordering: earliest timestamp first; ties broken by push order
    bool operator>(const Event& o) const {
        if (timestamp != o.timestamp) return timestamp > o.timestamp;
        return seq > o.seq;
    }
};
