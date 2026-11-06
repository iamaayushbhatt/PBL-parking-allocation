// Visualiser: the curses screen.
//   map on the left, metrics and log on the right, keys to spawn and remove cars.
// It reads the simulation only through Simulation's public functions and SimView.h.
//
// Files:  ui/Visualiser.cpp  (loop, map, status line)    -- Aayush
//         ui/Panel.cpp       (right panel, key handling)  -- Armaan
#ifndef VISUALISER_H
#define VISUALISER_H

#include <curses.h> // ncurses on Linux/macOS, PDCurses on Windows
#include <deque>
#include <string>

#include "SimView.h"
#include "Simulation.h"

// what the keys change; shared by the loop (Visualiser.cpp) and the keys (Panel.cpp)
struct UiState
{
    bool paused = false;
    bool quit = false;
    bool randomOn = false;
    int speedLevel = 2;       // index into the speed table in Panel.cpp
    unsigned randomSeed = 1;  // changes every time random traffic is switched on
};

// colours: always use uiColor(CP_...) so the screen still works on a terminal without colour
enum { CP_GREEN = 1, CP_YELLOW, CP_RED, CP_CYAN, CP_MAGENTA, CP_WHITE };
int uiColor(int pair);

// ---- Visualiser.cpp
int runVisualiser(Simulation &sim); // returns when the user presses q

// ---- Panel.cpp
double uiSpeed(const UiState &ui);                // simulated minutes per real second
void drawPanel(WINDOW *win, const SimSnapshot &snap, const std::deque<std::string> &log);
void handleKey(int key, Simulation &sim, UiState &ui);

#endif
