#pragma once
#include <queue>
#include <vector>
#include <functional>
#include <cstddef>
#include "Event.h"

// Min-heap of events ordered by timestamp (ties broken by push order).
class EventQueue {
    std::priority_queue<Event, std::vector<Event>, std::greater<Event>> pq;
    long long nextSeq = 0;

public:
    void pushEvent(Event e) { e.seq = nextSeq++; pq.push(e); }
    Event popNextEvent()    { Event e = pq.top(); pq.pop(); return e; }
    const Event& peek() const { return pq.top(); }
    bool empty() const { return pq.empty(); }
    std::size_t size() const { return pq.size(); }
};
