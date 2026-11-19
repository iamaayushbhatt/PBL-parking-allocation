# 🚗 Multi-Lane Parking Allocation and Rerouting Simulator

**An event-driven parking simulator in C++17 featuring intelligent slot allocation, congestion-aware routing, lane closures, reservations, live terminal visualization, parking tickets, and performance metrics.**


## 👥 Team Information

| Field | Details |
| --- | --- |
| **Team ID** | DSCPP-III-2026-T062 |
| **Team Name** | The Canvas |
| **Course** | B.Tech CSE |
| **Mentor** | Ms. Ashwini Yeole |

### Team Members

| Member | Student ID | Role | Main contribution |
| --- | --- | --- | --- |
| Aayush Bhatt (Team Lead) | 2510011035 | Team Lead & Integration | JSON layout loading, demo layout, `main.cpp`, integration |
| Armaan Sachdeva | 2510012259 | Data Structures | Event queue, dynamic lane weights/closures, interval tree, metrics, tests |
| Arihant Jain | 2510011114 | Algorithms | Dijkstra's algorithm, A* pathfinding, routing tests |
| Aman Aithani | 25101280977 | Object-Oriented Programming | Vehicle hierarchy, parking-slot states, tickets, admin metrics, dispatch/fee-related classes |

## 🎯 Problem Statement

Finding a suitable parking space in a large facility can waste time, fuel, and road capacity. A simple nearest-slot search is not enough: lanes may be congested or closed, slots may be reserved, and vehicle-specific needs (size, EV charging, accessibility) must be considered.

This project **simulates the full parking journey**, from vehicle arrival and slot selection to routing, parking, departure, fee calculation, and reporting. It is a software simulation rather than a sensor-connected physical parking system.

## ✨ Implemented Features

- **Event-driven simulation:** vehicle arrival, vehicle parked, vehicle exit, lane blocked, and lane reopened events.
- **Intelligent slot allocation:** selects a reachable, available slot compatible with the vehicle's size, EV requirement, accessibility requirement, and reservation window.
- **Graph-based navigation:** models junctions and lanes with capacities, occupancy, distances, and closure states.
- **Congestion-aware routing:** uses **A\*** to select the lowest-cost route among eligible slots; also implements **Dijkstra** for shortest-path computation and testing.
- **Closure-aware detours:** new arrivals avoid closed lanes. The report tracks routes that differ from their all-lanes-open alternative.
- **Interval-tree reservations:** detects overlapping time windows using half-open intervals `[start, end)`; the demo includes preconfigured bookings.
- **Vehicle and slot OOP models:** standard, electric, and accessible vehicles; compact, standard, and large spaces; slot states `FREE`, `RESERVED`, `OCCUPIED`, and `MAINTENANCE`.
- **Random scenarios:** seeded vehicle generation and timed lane closures for repeatable experiments.
- **Interactive terminal visualization:** a color-capable, keyboard-controlled map with live metrics and an event log (ncurses/PDCurses build).
- **Fees, tickets, and admin metrics:** records completed stays, calculates fees, displays total revenue, ticket count, and occupancy.
- **CSV export in console mode:** writes completed parking tickets to `tickets.csv` after the simulation finishes.
- **Performance and environmental estimates:** time to park, parking duration, distance-based estimated CO₂, and savings compared with a simulated baseline.

## 🧰 Tech Stack

| Category | Technology |
| --- | --- |
| Language | C++17 |
| Compiler | `g++` (C++17-capable) |
| Core concepts | OOP, graph algorithms, priority queues, event-driven simulation, interval trees |
| STL | `vector`, `map`, `set`, smart pointers, queues, random-number generation |
| Layout configuration | JSON, parsed by the project's own JSON parser |
| Optional UI library | **ncurses** (Linux/WSL) or **PDCurses** (native Windows) |
| Output | Console reports and CSV ticket history |
| Version control | Git and GitHub |

> The **text-only simulator does not require curses**. The **interactive screen does**. No third-party JSON parser is required.

## 🚀 Installation and Running

### 1. Clone the repository

```bash
git clone https://github.com/iamaayushbhatt/PBL-parking-allocation.git
cd PBL-parking-allocation
```

Run all build commands **from the repository root**, where `main.cpp` and `layouts/` are located.

### 2. Build the console simulator (Linux / macOS / WSL)

```bash
g++ -std=c++17 -I. \
  main.cpp Simulation.cpp scenario/RandomScenario.cpp \
  graph.cpp pathfinding.cpp intervaltree.cpp \
  models/Slot_State.cpp models/VehicleHierarchy.cpp models/Ticket_Metrics.cpp \
  metrics/metrics.cpp visualization/JsonParser.cpp visualization/layoutLoader.cpp \
  -o parking_sim
```

On **Windows with MinGW**, use this **one-line command in PowerShell** from the project directory:

```powershell
g++ -std=c++17 -I. main.cpp Simulation.cpp scenario/RandomScenario.cpp graph.cpp pathfinding.cpp intervaltree.cpp models/Slot_State.cpp models/VehicleHierarchy.cpp models/Ticket_Metrics.cpp metrics/metrics.cpp visualization/JsonParser.cpp visualization/layoutLoader.cpp -o parking_sim.exe
.\parking_sim.exe
```

The Linux command uses Bash line continuations (`\`); PowerShell does not interpret them the same way. If `g++` is not recognized, install/configure MinGW-w64 or use Ubuntu in WSL.

### 3. Run one of the console modes

**Demo scenario** (default layout and built-in vehicles):

```bash
./parking_sim
# Or explicitly:
./parking_sim layouts/DemoLayout.json
```

**Random scenario** (example: 30 vehicles, deterministic seed 42):

```bash
./parking_sim layouts/DemoLayout.json --random 30 --seed 42
```

The default seed is `1` if `--seed` is omitted. On Windows, use `./parking_sim.exe` or `.\parking_sim.exe` according to your shell.

### 4. Build and run the live terminal screen (Linux / WSL)

Install the screen dependency on Ubuntu / Debian / WSL:

```bash
sudo apt update
sudo apt install g++ libncurses-dev
```

Build with the screen-related source files, `-DWITH_UI`, and the ncurses linker flag:

```bash
g++ -std=c++17 -I. -DWITH_UI \
  main.cpp Simulation.cpp scenario/RandomScenario.cpp \
  graph.cpp pathfinding.cpp intervaltree.cpp \
  models/Slot_State.cpp models/VehicleHierarchy.cpp models/Ticket_Metrics.cpp \
  metrics/metrics.cpp visualization/JsonParser.cpp visualization/layoutLoader.cpp \
  ui/Visualiser.cpp ui/Panel.cpp \
  -lncurses -o parking_ui
```

Launch the interactive screen:

```bash
./parking_ui layouts/DemoLayout.json --ui
```

**Terminal size:** at least **84 columns × 20 rows**. A larger terminal is recommended to see the map and metrics panel comfortably.

**Windows users:** WSL with Ubuntu is the simplest documented option for the curses-based screen. Native Windows builds require a compatible **PDCurses** installation and linker setup (usually `-lpdcurses` instead of `-lncurses`). Simply compiling `--ui` without `-DWITH_UI` will print a message explaining that screen support was not included.

### 5. Interactive keyboard controls

| Key | Action |
| --- | --- |
| `S` | Spawn a standard vehicle requiring a standard-size slot |
| `E` | Spawn an electric vehicle |
| `A` | Spawn an accessible vehicle |
| `L` | Spawn a large standard vehicle |
| `C` | Spawn a compact standard vehicle |
| `X` | Make the longest-parked vehicle leave |
| `R` | Toggle generated random traffic and lane closures |
| `Space` | Pause / resume simulated time |
| `+` / `=` | Increase simulation speed |
| `-` / `_` | Decrease simulation speed |
| `Q` | Quit the screen |

Both uppercase and lowercase letters work. The display has a map area plus a live metrics/event-log panel. Speed choices currently range from **0.5 to 16 simulated minutes per real second**.

**Important:** The existing `tickets.csv` export happens in `Simulation::run()` (the completed **console** simulation). Quitting the interactive curses screen does **not currently call that export function**. If you need a CSV report, run demo or random **console** mode.

### What the interactive screen shows

The terminal UI is drawn in real time by the `ui/` module. It is **not a browser or graphical desktop application**: it uses a text-based lane map, colored symbols, a status bar, and a metrics/log panel.

```text
+------------------------ PARKING MAP -------------------------+---------------------------+
| IN -> + ---- + ---- + ---- + ---- + -> OUT                  | METRICS                   |
|          |      |      |                                   | Arrived / Parked / Exited |
|          +------+------ +                                  | Turned away / Rerouted    |
|          o  o   e  ~   o  a   ...                          | Avg. time / CO2 / Revenue |
|                                                              | LOG                       |
| o free   e EV   a accessible   ~ reserved   x closed        | Latest vehicle events ... |
+--------------------------------------------------------------+---------------------------+
 s standard  e EV  a accessible  l large  c compact  |  x exit  r random  space pause  q quit
```

*Illustrative terminal layout only, not a screenshot or verified run.* Actual junction positions, road shapes, counters and labels depend on `layouts/DemoLayout.json`, terminal dimensions, and simulation events.

**Map legend:** `o` = free regular slot, `e` = free EV slot, `a` = free accessible slot, `~` = reserved slot while a car travels, `C/E/A` = parked vehicle category, `=` = highlighted driving route, and `x` = closed lane. Traffic colors indicate lane occupancy when the terminal supports colors.


## 🧠 How the Simulator Works

```text
Read JSON lot layout
         |
         v
Create road graph, parking slots, reservations and event queue
         |
         v
Vehicle arrives -> identify vehicle category and size
         |
         v
Filter slots: free? big enough? correct EV/accessibility zone?
              booking window does not overlap?
         |
         v
Run A* from entry to each reachable, eligible slot
         |
         v
Choose slot with minimum congestion-weighted route cost
         |
         +------ No matching reachable slot --> Turn vehicle away
         |
         v
Reserve the chosen slot while the car travels
         |
         v
Process PARKED event -> mark slot occupied, update metrics
         |
         v
Process EXIT event -> free slot, calculate fee, record ticket
         |
         v
Console mode: print report and write tickets.csv
```

### Routing and lane closures

A parking layout is represented as a graph: **nodes** are road junctions, **edges** are drivable lanes, and parking slots attach to nodes. The graph uses dynamic costs influenced by congestion (lane occupancy relative to capacity). Closed lanes are excluded from routing.

For each arriving vehicle, the simulator checks eligible slots, runs A* to find a route, and chooses the slot with the lowest route cost. The A* heuristic uses junction coordinates. **Dijkstra** is also implemented and covered by routing tests.

When a lane is already closed at the time of a **new arrival**, the chosen route can detour around it. The `rerouted` metric compares that route with the route obtained if closed lanes had been open. **This is not yet a continuous mid-journey replan for vehicles already in transit.**

### Reservations

The interval-tree module stores reservation time ranges and checks for overlapping bookings. The simulated allocation step avoids slots reserved by another vehicle for the requested stay window. Reservation logic is implemented, but the current keyboard screen does **not** provide a form to create arbitrary user-entered bookings.

### Fee calculation

When a vehicle leaves, the simulator creates a `ParkingTicket` with its ID, registration, slot, floor, arrival/departure times, stay duration, and fee.

The **current provisional simulation fee rule** (implemented in `Simulation.cpp`) is. A source comment marks this helper for replacement when the separate fee calculator is integrated:

| Vehicle size | Base fee per started hour |
| --- | ---: |
| Compact | 20 |
| Standard | 30 |
| Large | 50 |

Electric vehicles add **10 per started hour**. Duration is rounded up to a full hour with a one-hour minimum. These are **simulated fee amounts**, not payment-gateway transactions; the current admin console labels revenue with `$`.

## 📊 Console Output and CSV Reports

Console runs print time-stamped activity logs, a metrics report, counts of **turned-away** and **rerouted** vehicles, and an **admin dashboard** with revenue, processed tickets, and final occupancy. A typical event log follows this format (placeholders below are not actual measured output):

```text
[t=  0.0] V1 (Standard, STANDARD) arrives
[t=  0.0]   -> slot <id>, route <node1>-><node2>->... (<distance> units, baseline <distance>)
[t= <time>] V1 parked in slot <id>  (occupancy <percentage>%)
[t= <time>] V1 leaves slot <id> after <minutes> min, fee <amount>
```

The CSV file is created in the directory where you launch the simulator:

```text
tickets.csv
```

Its header is:

```csv
TicketID,VehicleID,LicensePlate,SlotID,Floor,ArrivalTime,DepartureTime,Duration,TotalFee
```

One row is written per **completed vehicle exit**. The file is rewritten on each completed console simulation run; it is not a persistent database of every previous run. Values are in simulated minutes and simulated fee units.

## 🗂️ Project Structure

```text
PBL-parking-allocation/
├── main.cpp                    # CLI flags and selection of run mode
├── Simulation.h / .cpp         # Event handlers, allocation, fees, reporting
├── SimView.h                   # View/snapshot structures for the UI
├── Event.h / EventQueue.h      # Event definitions and priority queue
├── graph.h / .cpp              # Graph, lane occupancy and closure state
├── pathfinding.h / .cpp       # Dijkstra, A* and route calculations
├── intervaltree.h / .cpp      # Non-overlapping reservation windows
├── models/
│   ├── VehicleHierarchy.*     # Standard, electric, accessible vehicles
│   ├── Slot_State.*           # Parking slot information and state
│   └── Ticket_Metrics.*       # Tickets, admin revenue and CSV export
├── metrics/                    # Parking duration/time and CO2 estimates
├── scenario/                   # Seeded random vehicle/closure generation
├── visualization/              # JSON parser and layout loader
├── ui/
│   ├── Visualiser.*           # Terminal map and screen runtime
│   └── Panel.*                # Metrics panel and keyboard interactions
├── layouts/
│   └── DemoLayout.json        # Example parking facility layout
├── tests/                      # Unit and algorithm tests
├── report/                     # Academic phase reports
└── README.md
```

## 🗺️ Custom Layout Files

The simulator accepts a JSON layout file as its first path argument. The supplied `layouts/DemoLayout.json` is the recommended starting point.

| JSON key | What it represents |
| --- | --- |
| `nodes` | Junction IDs and their `x`, `y` coordinates |
| `lanes` | `from`, `to`, `distance`, `capacity`, and optional `twoWay` |
| `slots` | `id`, `node`, `floor`, `size`; optional `ev`, `accessible` |
| `entries` | Entry gate node IDs |
| `exits` | Exit gate node IDs |

Slot sizes are `COMPACT`, `STANDARD`, and `LARGE`. The custom parser validates the layout and reports invalid input. For layouts other than the bundled demo, **random** or **interactive** mode is generally a better starting point than the hard-coded demo schedule.

## ✅ Running Tests

From the repository root, build and run the existing assert-based tests:

```bash
g++ -std=c++17 -I. tests/test_graph.cpp graph.cpp -o t_graph && ./t_graph
g++ -std=c++17 -I. tests/test_intervaltree.cpp intervaltree.cpp -o t_interval && ./t_interval
g++ -std=c++17 -I. tests/test_eventqueue.cpp -o t_events && ./t_events
g++ -std=c++17 -I. tests/test_metrics.cpp metrics/metrics.cpp -o t_metrics && ./t_metrics
g++ -std=c++17 -I. tests/routing_test.cpp graph.cpp pathfinding.cpp -o t_routing && ./t_routing
```

Routing tests cover normal traffic, congestion, closed lanes, and an unreachable destination. Tests and UI functionality should be rerun on your own development machine after every integration change.

## 📌 Current Progress and Scope

| Area | Current code |
| --- | --- |
| Graph with congestion weights and lane closures | Implemented |
| Dijkstra and A* routing | Implemented |
| Interval-tree reservation checks | Implemented |
| Event-driven vehicle lifecycle | Implemented |
| Vehicle hierarchy and slot states | Implemented |
| JSON layout parsing and validation | Implemented |
| Fixed demo scenario | Implemented |
| Seeded random vehicles and closures | Implemented |
| Live curses-based terminal visualization | Implemented (optional build) |
| Ticket generation and admin revenue | Implemented |
| Console-mode ticket CSV export | Implemented |
| Full UI-based booking input | Not part of the current keyboard controls |
| Continuous rerouting of already-moving vehicles | Not implemented in the current event handler |
| Automatic ticket export when quitting UI | Not currently called by the UI entry point |

## 🔭 Possible Improvements

- Add a booking form or command for user-selected reservation slots and windows.
- Recalculate routes for cars already driving when a lane unexpectedly closes.
- Add export/report options directly inside the interactive UI.
- Add persistent storage (SQLite or another database) for tickets across runs.
- Add automated integration tests and a build script or CMake configuration.
- Replace simulated fee units with a configurable currency, rates, and tax rules.

## 📎 Repository

[GitHub — iamaayushbhatt/PBL-parking-allocation](https://github.com/iamaayushbhatt/PBL-parking-allocation)

---

**Academic project:** The Canvas · DSCPP-III-2026-T062
