
# 🚗 Multi-Lane Parking Allocation and Rerouting Simulator

### An Event-Driven Smart Parking Simulation System Using C++17, Data Structures and OOP

**Team:** The Canvas  
**Team ID:** DSCPP-III-2026-T062  
**Institution:** Graphic Era (Deemed to be University), Dehradun  
**Academic Session:** 2026–27  
**Course:** Project-Based Learning (Data Structures and OOP with C++)

---

## 📌 Project Overview

The **Multi-Lane Parking Allocation and Rerouting Simulator** is a C++17 application that models vehicle movement, intelligent parking-slot allocation, congestion-aware routing, lane closures, reservations, and parking management.

The project represents a parking facility as a weighted graph, where junctions are vertices and roads are edges.

When a vehicle enters the facility, the simulator:

1. Identifies suitable parking slots.
2. Checks slot availability and reservation conflicts.
3. Calculates an efficient route using graph algorithms.
4. Guides the vehicle toward its allocated slot.
5. Updates occupancy and traffic information.
6. Processes the vehicle's departure.
7. Calculates parking fees and records performance metrics.

An interactive terminal-based visualizer displays the parking layout, moving vehicles, occupancy, traffic statistics, environmental estimates, revenue, and simulation events.

The project is a **software simulation**, not a sensor-connected physical parking system.

---

## 🎯 Problem Statement

In large parking facilities, drivers often waste time searching for available spaces.

Common problems include:

- Difficulty locating suitable parking slots.
- Traffic congestion within parking facilities.
- Road or lane closures.
- Incompatible parking spaces for specific vehicles.
- Overlapping parking reservations.
- Unnecessary driving distance and fuel consumption.
- Inefficient parking utilization.

Our simulator demonstrates how graph algorithms, data structures, and object-oriented programming can address these challenges.

---

## 🎯 Project Objectives

- Model a parking facility using a weighted graph.
- Allocate suitable parking slots automatically.
- Implement Dijkstra's and A* pathfinding algorithms.
- Consider congestion when calculating routes.
- Avoid closed lanes during route selection.
- Prevent overlapping reservations using an interval tree.
- Manage vehicle arrivals and departures using an event queue.
- Support multiple vehicle and parking-slot categories.
- Display live simulation activity in a terminal interface.
- Generate parking tickets and calculate revenue.
- Measure occupancy, parking time, and estimated CO₂ emissions.

---

## 👥 Team Members and Contributions

| Team Member | Role | Contributions |
|---|---|---|
| Aayush Bhatt | Team Lead and Integration | JSON layout loader, application integration, main program, terminal visualizer, documentation |
| Armaan Sachdeva | Data Structures and Simulation | Event queue, AVL interval tree, simulation engine, congestion and closure handling, metrics |
| Arihant Jain | Graph and Pathfinding | Graph implementation, Dijkstra, A*, shortest-path calculations, routing tests |
| Aman Aithani | OOP and Parking Management | Vehicle hierarchy, slot states, ticket management, fees, dispatch-related models, admin metrics |

**Project Mentor:** Ms. Ashwini Yeole

---

## ✨ Key Features

### 1. Intelligent Parking Allocation

The simulator selects parking slots according to:

- Vehicle category and size.
- Slot availability.
- EV charging requirements.
- Accessibility requirements.
- Reservation conflicts.
- Route availability and cost.

If no suitable reachable slot is available, the vehicle is turned away.

### 2. Graph-Based Navigation

The parking facility is represented using an adjacency-list graph.

Each lane stores information such as:

- Destination node.
- Lane distance.
- Lane capacity.
- Current vehicle occupancy.
- Lane closure status.

### 3. Dijkstra and A* Algorithms

**Dijkstra's algorithm** calculates minimum-cost paths using nonnegative edge weights.

**A*** uses accumulated route cost and a heuristic to guide pathfinding.

The simulator uses A* in its slot-routing workflow, while Dijkstra is also implemented and tested.

### 4. Congestion-Aware Routing

Lane costs change according to traffic occupancy.

The routing model uses a congestion-dependent weight:

```text
Weight = ceil(distance × (1 + 2 × load / capacity))
```

A congested lane receives a higher routing cost, encouraging the algorithm to consider alternative paths.

### 5. Lane Closures

The graph supports marking lanes as closed or reopened.

Closed lanes are excluded when calculating new routes.

The current implementation supports closure-aware routing for new arrivals. Continuous replanning for vehicles already traveling is a possible future enhancement.

### 6. Reservation Management

An AVL-balanced interval tree manages parking reservation windows.

Reservations use half-open intervals:

```text
[start, end)
```

This prevents overlapping bookings for the same parking slot.

### 7. Event-Driven Simulation

The simulator uses a priority queue to process events in chronological order.

Events include:

- Vehicle arrival.
- Vehicle parking.
- Vehicle departure or exit.
- Lane closure.
- Lane reopening.

### 8. Vehicle and Slot Models

The project uses object-oriented programming to represent different vehicle categories and parking slots.

Vehicle categories include standard, electric, and accessible vehicles.

Parking slots support different sizes and special requirements.

Slot states include:

```text
FREE
RESERVED
OCCUPIED
MAINTENANCE
```

### 9. Interactive Terminal Visualization

The live terminal interface displays:

- Parking roads and junctions.
- Entry and exit gates.
- Available and occupied slots.
- Moving vehicles.
- Vehicle routes.
- Lane congestion and closures.
- Parking occupancy.
- Traffic statistics.
- Revenue and environmental metrics.
- Time-stamped event logs.

The UI is implemented using **PDCurses on Windows** and **ncurses on Linux**.

### 10. Random Traffic Simulation

The simulator supports seeded random traffic scenarios for repeatable experiments.

### 11. Parking Tickets and Revenue

The system records completed parking stays and generates ticket information.

Ticket data includes:

- Ticket ID.
- Vehicle ID.
- License plate.
- Parking slot.
- Floor number.
- Arrival time.
- Departure time.
- Parking duration.
- Total fee.

### 12. Performance Metrics

The simulator tracks:

- Total arrivals.
- Successfully parked vehicles.
- Exited vehicles.
- Turned-away vehicles.
- Rerouted vehicles.
- Occupancy percentage.
- Average parking time.
- Average parking duration.
- Estimated CO₂ emitted and saved.
- Total parking revenue.

---

## 🛠️ Technology Stack

| Category | Technology |
|---|---|
| Programming Language | C++17 |
| Core Concepts | Data Structures, Algorithms, OOP |
| Data Structures | Graph, priority queue, AVL interval tree, maps, sets |
| Algorithms | Dijkstra, A*, congestion-aware routing |
| Standard Library | C++ STL |
| Terminal UI | PDCurses / ncurses |
| Layout Input | JSON |
| Ticket Output | CSV |
| Compiler | GCC / G++ |
| Development Environment | MSYS2 UCRT64, VS Code |
| Version Control | Git and GitHub |
| Operating Systems | Windows and Linux |

**Database:** None. Active simulation data is maintained in memory.

---

## 🏗️ System Architecture

```text
                JSON Layout
                     |
                     v
             JSON Parser / Loader
                     |
                     v
              Parking Graph
                     |
                     v
              Simulation Engine
                     |
        +------------+------------+
        |            |            |
        v            v            v
    Event Queue   Slot Models   Reservations
        |            |            |
        +------------+------------+
                     |
                     v
           Slot Selection and A*
                     |
                     v
             Vehicle Movement
                     |
                     v
             Parking / Departure
                     |
            +--------+--------+
            |                 |
            v                 v
       Live Terminal      Metrics / Tickets
            UI                 |
                               v
                          CSV Export
                       (Console Mode)
```

The simulation engine and visualizer are separated.

The visualizer reads simulation snapshots, allowing the core engine to run without an interactive screen.

---

## 📂 Project Structure

```text
PBL-parking-allocation/
|
|-- main.cpp
|-- Simulation.cpp
|-- Simulation.h
|-- SimView.h
|
|-- Event.h
|-- EventQueue.h
|
|-- graph.cpp
|-- graph.h
|-- pathfinding.cpp
|-- pathfinding.h
|
|-- intervaltree.cpp
|-- intervaltree.h
|
|-- models/
|   |-- VehicleHierarchy.cpp
|   |-- VehicleHierarchy.h
|   |-- Slot_State.cpp
|   |-- Slot_State.h
|   |-- Ticket_Metrics.cpp
|   |-- Ticket_Metrics.h
|   |-- Dispatch_Fee.cpp
|
|-- metrics/
|   |-- metrics.cpp
|
|-- scenario/
|   |-- RandomScenario.cpp
|
|-- visualization/
|   |-- JsonParser.cpp
|   |-- layoutLoader.cpp
|
|-- ui/
|   |-- Visualiser.cpp
|   |-- Panel.cpp
|
|-- layouts/
|   |-- DemoLayout.json
|   |-- GridLayout.json
|
|-- tests/
|   |-- test_graph.cpp
|   |-- test_intervaltree.cpp
|   |-- test_eventqueue.cpp
|   |-- test_metrics.cpp
|   |-- routing_test.cpp
|   |-- test_simulation.cpp
|
|-- README.md
```

The tree highlights the main source files; other layouts, reports, headers, and tests may also be present.

---

## 🚀 Installation and Execution

### Prerequisites

For Windows:

- MSYS2 UCRT64
- GCC / G++
- PDCurses
- Git

For Linux:

- GCC / G++
- ncurses development library

### Step 1: Clone the Repository

```bash
git clone https://github.com/iamaayushbhatt/PBL-parking-allocation.git

cd PBL-parking-allocation
```

### Step 2: Install Dependencies on Windows

Open the **MSYS2 UCRT64** terminal.

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-ucrt-x86_64-make
pacman -S mingw-w64-ucrt-x86_64-pdcurses
```

### Step 3: Build the Interactive Simulator on Windows

Run the following command from the repository root:

```bash
g++ -std=c++17 -DWITH_UI -DPDC_FORCE_UTF8 \
-I. -Imodels -I/ucrt64/include/pdcurses \
main.cpp graph.cpp pathfinding.cpp intervaltree.cpp \
Simulation.cpp metrics/metrics.cpp \
models/Dispatch_Fee.cpp models/Slot_State.cpp \
models/Ticket_Metrics.cpp models/VehicleHierarchy.cpp \
scenario/RandomScenario.cpp ui/Panel.cpp ui/Visualiser.cpp \
visualization/JsonParser.cpp visualization/layoutLoader.cpp \
-o parking_sim.exe -lpdcurses_wincon -lwinmm
```

This command uses the PDCurses configuration used for the native Windows UI build.

### Step 4: Run the Demo Layout

```bash
./parking_sim.exe layouts/DemoLayout.json --ui
```

### Step 5: Run the Large Grid Layout

```bash
./parking_sim.exe layouts/GridLayout.json --ui
```

The grid layout provides a larger parking network with 69 parking slots.

### Step 6: Run a Random Scenario

```bash
./parking_sim.exe layouts/DemoLayout.json --random 30 --seed 42
```

This creates a reproducible random scenario using the specified seed.

---

## 🐧 Linux / WSL Build

Install dependencies:

```bash
sudo apt update
sudo apt install g++ libncurses-dev
```

Compile the interactive simulator:

```bash
g++ -std=c++17 -DWITH_UI -I. -Imodels \
main.cpp graph.cpp pathfinding.cpp intervaltree.cpp \
Simulation.cpp metrics/metrics.cpp \
models/Dispatch_Fee.cpp models/Slot_State.cpp \
models/Ticket_Metrics.cpp models/VehicleHierarchy.cpp \
scenario/RandomScenario.cpp ui/Panel.cpp ui/Visualiser.cpp \
visualization/JsonParser.cpp visualization/layoutLoader.cpp \
-o parking_ui -lncurses
```

Run:

```bash
./parking_ui layouts/DemoLayout.json --ui
```

The terminal should be large enough to display both the parking map and statistics panel.

---

## 🎮 Interactive UI Controls

| Key | Action |
|---|---|
| `S` | Spawn a standard vehicle |
| `E` | Spawn an electric vehicle |
| `A` | Spawn an accessible vehicle |
| `L` | Spawn a large vehicle |
| `C` | Spawn a compact vehicle |
| `X` | Make the longest-parked vehicle leave |
| `R` | Toggle random traffic generation |
| `Space` | Pause or resume simulation |
| `+` / `=` | Increase simulation speed |
| `-` / `_` | Decrease simulation speed |
| `M` | Toggle monochrome display |
| `Q` | Quit the simulator |

The UI includes a parking map and live metrics/event-log panels.

**Monochrome mode** changes the appearance of the interface without changing simulation behavior.

---

## 🖥️ Understanding the Live UI

### Parking Map

Displays the road network, junctions, gates, parking slots, and vehicles.

### LOT Panel

Displays parking capacity, occupied slots, and occupancy percentage.

### TRAFFIC Panel

Displays arrivals, parked vehicles, exits, moving vehicles, reroutes, and turn-aways.

### RESULTS Panel

Displays average parking time, average stay, estimated CO₂ emissions, estimated CO₂ savings, and revenue.

### LOG Panel

Displays time-stamped simulation events.

Example:

```text
[t=694.0] R31 (Standard, LARGE) arrives
[t=694.0] -> slot 27, route 0->14->10->6
[t=708.5] R31 parked in slot 27
```

The event log helps users understand how vehicles are allocated and routed.

---

## 🧠 Algorithms and Data Structures

### Weighted Graph

The graph stores parking lanes and junctions.

Each edge can contain distance, capacity, vehicle count, and closure information.

### Dijkstra's Algorithm

Finds minimum-cost paths using nonnegative edge weights.

A priority-queue implementation typically runs in:

```text
O((V + E) log V)
```

### A* Algorithm

Uses:

```text
f(n) = g(n) + h(n)
```

Where:

- `g(n)` is the accumulated path cost.
- `h(n)` is the heuristic estimate.
- `f(n)` is the estimated total cost.

The project uses a Manhattan-distance-based heuristic.

### Min-Heap Event Queue

Schedules simulation events by time.

This avoids repeatedly scanning all scheduled events to find the next event.

### AVL-Balanced Interval Tree

Stores reservation intervals and helps detect conflicting bookings.

AVL rotations keep the tree balanced.

---

## 💾 JSON Input and CSV Output

### JSON

JSON files describe the parking facility.

They contain information about:

- Junctions.
- Roads and lane distances.
- Lane capacities.
- Parking slots.
- Slot types.
- Entry and exit gates.

### CSV

Completed console simulations export ticket records to:

```text
tickets.csv
```

Example header:

```csv
TicketID,VehicleID,LicensePlate,SlotID,Floor,ArrivalTime,DepartureTime,Duration,TotalFee
```

**Important:** The current interactive UI does not automatically export `tickets.csv` when the user quits. Use a completed console-mode simulation for CSV output.

---

## 🧪 Testing and Validation

The project contains tests for its core modules.

### Graph Test

```bash
g++ -std=c++17 -I. tests/test_graph.cpp graph.cpp -o t_graph
./t_graph
```

### Interval Tree Test

```bash
g++ -std=c++17 -I. tests/test_intervaltree.cpp intervaltree.cpp -o t_interval
./t_interval
```

### Event Queue Test

```bash
g++ -std=c++17 -I. tests/test_eventqueue.cpp -o t_events
./t_events
```

### Metrics Test

```bash
g++ -std=c++17 -I. tests/test_metrics.cpp metrics/metrics.cpp -o t_metrics
./t_metrics
```

### Routing Test

```bash
g++ -std=c++17 -I. tests/routing_test.cpp graph.cpp pathfinding.cpp -o t_routing
./t_routing
```

Routing test scenarios include:

1. Normal traffic.
2. Congested lanes.
3. Closed lanes.
4. Unreachable destinations.

Tests should be rerun after changes to the source code.

---

## 📊 Performance and Results

The following results were reported in the project's evaluation presentation.

### Demo Scenario

| Metric | Result |
|---|---:|
| Total arrivals | 13 |
| Successfully parked | 11 |
| Turned away | 2 |
| Rerouted | 5 |
| Average time to park | 7.5 minutes |
| Average parking duration | 37.7 minutes |
| Estimated CO₂ emitted | 15.9 g |
| Estimated CO₂ saved | 2.7 g |
| Simulated revenue | 875.40 |

### Grid Scenario

**Configuration:** 80 vehicles, seed 5.

| Metric | Result |
|---|---:|
| Total arrivals | 80 |
| Successfully parked | 66 |
| Turned away | 14 |
| Estimated CO₂ emitted | 179.2 g |
| Estimated CO₂ saved | 139.6 g |
| Simulated revenue | 8737.35 |

These figures are scenario-specific simulation results and are not live measurements.

Actual results depend on the selected layout, traffic generation, random seed, and implementation version.

---

## 📈 Current Implementation Status

| Module | Status |
|---|---|
| Weighted graph | Implemented |
| Dijkstra's algorithm | Implemented |
| A* pathfinding | Implemented |
| Congestion-aware routing | Implemented |
| Lane closures | Implemented |
| AVL interval tree | Implemented |
| Event queue | Implemented |
| Vehicle and slot models | Implemented |
| JSON layout loader | Implemented |
| Random traffic scenarios | Implemented |
| Interactive terminal UI | Implemented |
| Parking tickets and metrics | Implemented |
| Console CSV export | Implemented |

### Known Limitations

- The current engine uses a single entry gate.
- The simulator uses modeled data rather than live sensors.
- Interactive manual reservation entry is not available.
- Vehicles already moving are not continuously replanned after a lane closure.
- Quitting the interactive UI does not automatically export ticket CSV data.
- Terminal dimensions and PDCurses compatibility can affect the display.

---

## 🔮 Future Scope

Potential improvements include:

- Multiple entry and exit gates.
- Multi-floor parking facilities.
- Live parking sensor integration.
- Interactive reservation booking.
- Immediate rerouting for moving vehicles.
- Automatic UI-mode CSV export.
- Persistent database storage.
- A graphical desktop or web interface.
- More advanced traffic prediction.
- Configurable parking fees and reporting.

---

## 🎓 Academic Concepts Demonstrated

### Data Structures

- Graphs and adjacency lists.
- Priority queues and binary heaps.
- AVL trees and interval trees.
- Maps and sets.

### Algorithms

- Dijkstra's shortest-path algorithm.
- A* pathfinding.
- Congestion-aware route selection.
- Reservation-overlap checking.
- Event scheduling.

### Object-Oriented Programming

- Classes and objects.
- Encapsulation.
- Inheritance.
- Polymorphism.
- Constructors.
- Smart pointers.
- STL containers.

---

## 🔗 GitHub Repository

**Repository:**  
https://github.com/iamaayushbhatt/PBL-parking-allocation

---

## 🏁 Conclusion

The Multi-Lane Parking Allocation and Rerouting Simulator demonstrates how data structures, graph algorithms, object-oriented programming, and event-driven simulation can work together in a practical software project.

It supports intelligent slot allocation, congestion-aware routing, reservation checking, vehicle movement, parking management, and performance analysis.

The interactive terminal interface makes these algorithms observable through a working parking simulation.

---

**Developed by Team The Canvas**  
**Graphic Era (Deemed to be University), Dehradun**
