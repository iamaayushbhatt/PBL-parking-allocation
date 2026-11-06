// The right-hand panel (metrics and log) and what each key does.
#include "ui/Visualiser.h"

#include <algorithm>
#include <cstdio>

// simulated minutes per real second; + and - move along this list
static const double SPEEDS[] = {0.5, 1, 2, 4, 8, 16};
static const int SPEED_COUNT = (int)(sizeof(SPEEDS) / sizeof(SPEEDS[0]));

double uiSpeed(const UiState &ui)
{
    return SPEEDS[std::max(0, std::min(ui.speedLevel, SPEED_COUNT - 1))];
}

// ----------------------------------------------------------------- the panel
void drawPanel(WINDOW *win, const SimSnapshot &snap, const std::deque<std::string> &log)
{
    int h, w;
    getmaxyx(win, h, w);
    werase(win);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+'); // plain ASCII: works in every terminal

    const MetricsView &m = snap.metrics;
    int textWidth = std::max(0, w - 2);
    char line[128];
    int row = 1;

    auto put = [&](const char *text, int attr)
    {
        if (row >= h - 1)
            return;
        wattron(win, attr);
        mvwaddnstr(win, row, 1, text, textWidth);
        wattroff(win, attr);
        row++;
    };

    put("METRICS", A_BOLD | uiColor(CP_CYAN));
    snprintf(line, sizeof line, "Arrived       %6d", m.arrived);
    put(line, 0);
    snprintf(line, sizeof line, "Parked        %6d", m.parked);
    put(line, 0);
    snprintf(line, sizeof line, "Exited        %6d", m.exited);
    put(line, 0);
    snprintf(line, sizeof line, "Turned away   %6d", m.turnedAway);
    put(line, m.turnedAway > 0 ? uiColor(CP_YELLOW) : 0);
    snprintf(line, sizeof line, "Rerouted      %6d", m.rerouted);
    put(line, 0);
    snprintf(line, sizeof line, "Avg to park   %6.1f min", m.avgTimeToPark);
    put(line, 0);
    snprintf(line, sizeof line, "Avg stay      %6.1f min", m.avgDuration);
    put(line, 0);
    snprintf(line, sizeof line, "CO2 emitted   %6.1f g", m.totalCO2);
    put(line, 0);
    snprintf(line, sizeof line, "CO2 saved     %6.1f g", m.co2Saved);
    put(line, uiColor(CP_GREEN));
    snprintf(line, sizeof line, "Revenue     %8.2f", m.revenue);
    put(line, 0);

    row++; // blank line
    put("LOG", A_BOLD | uiColor(CP_CYAN));

    // newest lines at the bottom: show as many as fit
    int room = (h - 1) - row;
    if (room <= 0)
        return;
    size_t first = log.size() > (size_t)room ? log.size() - room : 0;
    for (size_t i = first; i < log.size(); i++)
    {
        const std::string &text = log[i];
        int attr = 0;
        if (text.find("turned away") != std::string::npos)
            attr = uiColor(CP_YELLOW);
        else if (text.find("LANE") != std::string::npos)
            attr = uiColor(CP_RED);
        else if (text.find("REROUTED") != std::string::npos)
            attr = uiColor(CP_MAGENTA);
        put(text.c_str(), attr);
    }
}

// ------------------------------------------------------------------- the keys
void handleKey(int key, Simulation &sim, UiState &ui)
{
    switch (key)
    {
    case 's': case 'S': sim.spawnVehicle("Standard", "STANDARD"); break;
    case 'e': case 'E': sim.spawnVehicle("Electric", "STANDARD"); break;
    case 'a': case 'A': sim.spawnVehicle("Accessible", "STANDARD"); break;
    case 'l': case 'L': sim.spawnVehicle("Standard", "LARGE"); break;
    case 'c': case 'C': sim.spawnVehicle("Standard", "COMPACT"); break;

    case 'x': case 'X': sim.exitLongestParked(); break; // does nothing if nobody is parked

    case 'r': case 'R':
        ui.randomOn = !ui.randomOn;
        sim.setRandomTraffic(ui.randomOn, ui.randomSeed++);
        break;

    case ' ': ui.paused = !ui.paused; break;
    case '+': case '=': ui.speedLevel = std::min(ui.speedLevel + 1, SPEED_COUNT - 1); break;
    case '-': case '_': ui.speedLevel = std::max(ui.speedLevel - 1, 0); break;

    case 'q': case 'Q': ui.quit = true; break;
    default: break;
    }
}
