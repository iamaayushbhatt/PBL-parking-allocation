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

// colours: always use uiColor(CP_...) or uiBlock(CP_...) so the screen still works on a terminal
// without colour.
//   CP_GREEN .. CP_WHITE      coloured text on black
//   CP_ROAD .. CP_KEY         solid coloured blocks (roads, routes, cars, bars, key caps)
enum
{
    CP_GREEN = 1, CP_YELLOW, CP_RED, CP_CYAN, CP_MAGENTA, CP_WHITE,
    CP_ROAD,      // clear road: black on grey
    CP_ROAD_BUSY, // half-full lane: black on yellow (also the "paused" badge)
    CP_ROAD_FULL, // full lane: white on red
    CP_ROUTE,     // route of a driving car: white on blue
    CP_CAR_STD,   // standard car: black on green (also the "running" badge)
    CP_CAR_EV,    // electric car: black on cyan
    CP_CAR_ACC,   // accessible car: white on magenta
    CP_BAR,       // top and bottom bar: black on grey
    CP_KEY        // key caps in the bottom bar: black on cyan
};
// Attributes are chtype, never int: with a 64-bit chtype (PDCurses on Windows) the colour pair
// lives above bit 31 and an int silently drops it.
extern bool uiMono;          // key m: draw without colour (for terminals that do not show it)
bool uiHasColor();           // true if colour is available and not switched off
chtype uiColor(int pair);    // colour pair for text, 0 without colour
chtype uiBlock(int pair);    // colour pair for a solid block, reverse video without colour

// ---- Visualiser.cpp
int runVisualiser(Simulation &sim); // returns when the user presses q

// ---- Panel.cpp
double uiSpeed(const UiState &ui);                // simulated minutes per real second
void drawPanel(WINDOW *win, const SimSnapshot &snap, const std::deque<std::string> &log);
void handleKey(int key, Simulation &sim, UiState &ui);

#endif

