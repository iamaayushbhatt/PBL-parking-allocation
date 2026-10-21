# Multi-Lane Parking Allocation and Rerouting Simulator

## 👥TEAM INFORMATION

**Team ID:** DSCPP-III-2026-T062 

**Team Name:** The Canvas

**Course:** BTech CSE

**Mentor:** Ms. Ashwini Yeole


---

## 1. 🔴Problem Statement / Objective
An event-driven parking lot simulator written in C and C++. Vehicles arrive at a multi-lane parking lot, and are assigned the best suitable free slot, and are routed to it with A\* pathfinding. Routes avoid crowded and closed lanes. The simulator reports time to park, estimated CO2 and parking tickets with fees.

Drivers lose time and fuel circling a crowded lot for a spot that fits their vehicle. Roaming for parking is a well-known contributor to urban congestion and emissions.

**Objective:** simulate a parking facility that dynamically allocates the nearest suitable slot and reroutes vehicles around congestion, using event-driven simulation and OOP design in C++.

**Specific goals:**

1. Run an event-driven simulation (arrivals, parking, exits, lane closures) over a lot layout loaded from JSON.
2. Guide each vehicle to the nearest suitable slot with A\* and dynamic lane weights.
3. Support minute-level reservation windows with no overlaps, using an interval tree.
4. Assign vehicles to EV and accessible zones, and to slots of the right size.
5. Generate tickets and fees by vehicle type and duration, and measure time to park and CO2.

## 2. 👥Team Members

| Member | Student ID | Role | Main contribution |
|---|---|---|---|
| Aayush Bhatt (Team Lead) | 2510011035 | Team Lead & Integration | JSON layout loader, demo layout, orchestrator (`main.cpp`) |
| Armaan Sachdeva | 2510012259 | Data structures | Event queue, dynamic lane weights and closures, interval tree, metrics, unit tests |
| Arihant Jain | 2510011114 | Algorithms | Pathfinding (Dijkstra, A\*), routing tests |
| Aman Aithani | 25101280977 | OOP | Vehicle hierarchy, slot states, tickets, admin metrics, dispatch/fee classes |

## 3.⚙️ Technologies / Tools Used

- **Language:** C++17 and C
- **Compiler:** `g++` with `-std=c++17`
- **Input format:** JSON lot layouts, read by a hand-written parser (no external library such as nlohmann/json)
- **Version control:** Git and GitHub
- **Testing:** assert-based unit tests in `tests/`

## 4. 📦Project Setup / Installation

Requirements: a C++17 compiler (`g++` 9 or newer) on Linux, macOS or Windows (MinGW/WSL). No other dependencies.

```bash
git clone https://github.com/iamaayushbhatt/PBL-parking-allocation.git
cd PBL-parking-allocation
```

**Build the simulator** (from the repo root):

```bash
g++ -std=c++17 -I. main.cpp graph.cpp pathfinding.cpp intervaltree.cpp \
    models/Slot_State.cpp models/VehicleHierarchy.cpp models/Ticket_Metrics.cpp \
    metrics/metrics.cpp visualization/JsonParser.cpp visualization/layoutLoader.cpp \
    -o parking_sim
```

**Run:**

```bash
./parking_sim layouts/DemoLayout.json
```

If no path is given, it uses `layouts/DemoLayout.json`.

**Run the unit tests** (from the repo root):

```bash
g++ -std=c++17 -I. tests/test_graph.cpp graph.cpp -o t_graph && ./t_graph
g++ -std=c++17 -I. tests/test_intervaltree.cpp intervaltree.cpp -o t_interval && ./t_interval
g++ -std=c++17 -I. tests/test_eventqueue.cpp -o t_events && ./t_events
g++ -std=c++17 -I. tests/test_metrics.cpp metrics/metrics.cpp -o t_metrics && ./t_metrics
g++ -std=c++17 -I. tests/routing_test.cpp graph.cpp pathfinding.cpp -o t_routing && ./t_routing
```

### Custom layouts

A layout is a JSON file with these keys:

| Key | Meaning |
|---|---|
| `nodes` | Junctions: `id`, `x`, `y` (coordinates are used by A\*) |
| `lanes` | Roads: `from`, `to`, `distance`, `capacity`, optional `twoWay` |
| `slots` | Parking slots: `id`, `node`, `floor`, `size` (`COMPACT`/`STANDARD`/`LARGE`), optional `ev`, `accessible` |
| `entries`, `exits` | Gate nodes (at least one of each) |

See `layouts/DemoLayout.json` for a working example. Invalid files are rejected with a readable error message.

## 5. 📚 Modules / Major Features 

| Module | Files | What it does |
|---|---|---|
| Navigation graph | `graph.h/.cpp` | Directed lane graph. Lane cost is `ceil(distance * (1 + 2 * vehicles / capacity))`, so busier lanes cost more. Lanes can be closed and reopened. |
| Pathfinding | `pathfinding.h/.cpp` | Dijkstra and A\* (Manhattan heuristic) over the congestion-weighted graph, plus route cost. |
| Reservations | `intervaltree.h/.cpp` | Interval tree on half-open windows `[low, high)`: insert, remove, overlap query, `tryReserve`. |
| Event engine | `Event.h`, `EventQueue.h` | Min-heap ordered by timestamp, ties broken by push order. Events: arrival, parked, exit, lane blocked, lane opened. |
| Vehicle and slot models | `models/` | `Vehicle` base class with `StandardVehicle`, `ElectricVehicle`, `AccessibleVehicle`. `ParkingSlot` with FREE, OCCUPIED, RESERVED, MAINTENANCE states, size, EV and accessible zones. |
| Tickets and admin | `models/Ticket_Metrics.*` | `ParkingTicket`, and an admin dashboard with revenue, ticket count and occupancy. |
| Metrics | `metrics/` | Time to park, parking duration, CO2 emitted and CO2 saved against an unguided baseline (average distance to all suitable free slots). |
| Layout input | `visualization/` | Hand-written JSON parser and layout loader with validation. |
| Orchestrator | `main.cpp` | Runs the simulation: filters slots, picks the lowest-cost route, reroutes around closed lanes, records metrics and tickets, prints the report. |

**How a vehicle is allocated:** a slot is suitable if it is free, large enough, matches the vehicle's EV/accessible need, and has no conflicting booking in the vehicle's stay window. Among suitable slots, the one with the lowest congestion-weighted A\* route cost is chosen. If none is reachable or suitable, the vehicle is turned away.

**Repository layout**

```
main.cpp                 simulation orchestrator
graph.*  pathfinding.*   lane graph and routing
intervaltree.*           reservations
Event.h  EventQueue.h    event engine
models/                  vehicles, slots, tickets, dispatch/fee
metrics/                 time-to-park and emissions
visualization/           JSON parser and layout loader
layouts/                 lot layouts (DemoLayout.json)
tests/                   unit tests
report/                  Phase reports
```

## 6. 📋Current Project Status / Progress
ㅤ

| Area | Status |
|---|---|
| Graph, congestion weights, lane closures | Completed |
| Dijkstra and A\* pathfinding | Completed |
| Interval tree reservations | Completed |
| Event queue | Completed |
| Vehicle and slot models | Completed |
| JSON layout loader | Completed |
| Metrics (time to park, CO2) | Completed |
| Simulation orchestrator (demo scenario) | Completed |
| Tickets and admin dashboard | In progress|
| Random arrivals and user bookings | Planned |
| Visualization | Planned |
| Storing ticket information in CSV file | Planned |


