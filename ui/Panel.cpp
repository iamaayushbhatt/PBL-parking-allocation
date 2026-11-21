// The right-hand panel (metrics and log) and what each key does.
#include "ui/Visualiser.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

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
    const int textWidth = std::max(0, w - 2);
    int row = 1;
    char buf[64];

    auto text = [&](const std::string &s, chtype attr)
    {
        if (row >= h - 1)
            return;
        wattron(win, attr);
        mvwaddnstr(win, row, 1, s.c_str(), textWidth);
        wattroff(win, attr);
        row++;
    };

    // "TITLE ---------": bold title, then a rule to the edge
    auto title = [&](const char *t)
    {
        if (row >= h - 1)
            return;
        wattron(win, A_BOLD | uiColor(CP_CYAN));
        mvwaddnstr(win, row, 1, t, textWidth);
        wattroff(win, A_BOLD | uiColor(CP_CYAN));
        for (int x = 2 + (int)strlen(t); x < w - 1; x++)
            mvwaddch(win, row, x, '-');
        row++;
    };

    // one cell: label on the left, value on the right, `width` characters in all
    auto cell = [](const std::string &label, const std::string &value, int width)
    {
        int gap = std::max(1, width - (int)label.size() - (int)value.size());
        return label + std::string(gap, ' ') + value;
    };

    // two cells side by side (one per line if the panel is too narrow)
    auto pair = [&](const char *l1, int v1, const char *l2, int v2, chtype attr2)
    {
        if (textWidth >= 30)
        {
            int cw = (textWidth - 2) / 2;
            std::string left = cell(l1, std::to_string(v1), cw);
            std::string right = cell(l2, std::to_string(v2), cw);
            if (row >= h - 1)
                return;
            mvwaddnstr(win, row, 1, left.c_str(), cw);
            wattron(win, attr2);
            mvwaddnstr(win, row, 1 + cw + 2, right.c_str(), cw);
            wattroff(win, attr2);
            row++;
        }
        else
        {
            text(cell(l1, std::to_string(v1), textWidth), 0);
            text(cell(l2, std::to_string(v2), textWidth), attr2);
        }
    };

    // ---- how full is the lot
    int total = (int)snap.slots.size(), inUse = 0, driving = 0;
    for (const SlotView &s : snap.slots)
        if (s.occupied)
            inUse++;
    for (const CarView &c : snap.cars)
        if (!c.parked)
            driving++;
    double frac = total > 0 ? (double)inUse / total : 0;

    title("LOT");
    snprintf(buf, sizeof buf, "%d / %d", inUse, total);
    text(cell("Slots in use", buf, textWidth), 0);
    if (row < h - 1)
    {
        int barW = std::max(0, textWidth - 6);
        int fill = (int)(frac * barW + 0.5);
        chtype attr = frac >= 0.9 ? uiColor(CP_RED) : frac >= 0.6 ? uiColor(CP_YELLOW) : uiColor(CP_GREEN);
        mvwaddch(win, row, 1, '[');
        wattron(win, attr | A_BOLD);
        for (int i = 0; i < barW; i++)
            mvwaddch(win, row, 2 + i, i < fill ? '#' : '.');
        wattroff(win, attr | A_BOLD);
        mvwaddch(win, row, 2 + barW, ']');
        snprintf(buf, sizeof buf, "%3d%%", (int)(frac * 100 + 0.5));
        mvwaddnstr(win, row, 3 + barW, buf, std::max(0, w - 2 - (3 + barW) + 1));
        row++;
    }

    // ---- counters
    title("TRAFFIC");
    pair("Arrived", m.arrived, "Parked", m.parked, 0);
    pair("Exited", m.exited, "Driving", driving, 0);
    pair("Rerouted", m.rerouted, "Turned away", m.turnedAway, m.turnedAway > 0 ? uiColor(CP_YELLOW) | A_BOLD : 0);

    // ---- averages, CO2 and money
    title("RESULTS");
    snprintf(buf, sizeof buf, "%.1f min", m.avgTimeToPark);
    text(cell("Avg time to park", buf, textWidth), 0);
    snprintf(buf, sizeof buf, "%.1f min", m.avgDuration);
    text(cell("Avg stay", buf, textWidth), 0);
    snprintf(buf, sizeof buf, "%.1f g", m.totalCO2);
    text(cell("CO2 emitted", buf, textWidth), 0);
    snprintf(buf, sizeof buf, "%.1f g", m.co2Saved);
    text(cell("CO2 saved", buf, textWidth), uiColor(CP_GREEN) | A_BOLD);
    snprintf(buf, sizeof buf, "%.2f", m.revenue);
    text(cell("Revenue", buf, textWidth), A_BOLD);

    // ---- log: long lines wrap, newest at the bottom, as many rows as fit
    title("LOG");
    int room = (h - 1) - row;
    if (room <= 0)
        return;

    struct LogRow
    {
        std::string text;
        chtype attr;
    };
    std::vector<LogRow> rowsOut;
    size_t from = log.size() > 80 ? log.size() - 80 : 0; // more than enough for any window
    for (size_t i = from; i < log.size(); i++)
    {
        const std::string &t = log[i];
        chtype attr = 0;
        if (t.find("turned away") != std::string::npos)
            attr = uiColor(CP_YELLOW) | A_BOLD;
        else if (t.find("LANE") != std::string::npos)
            attr = uiColor(CP_RED) | A_BOLD;
        else if (t.find("REROUTED") != std::string::npos)
            attr = uiColor(CP_MAGENTA) | A_BOLD;
        else if (t.find("parked in") != std::string::npos)
            attr = uiColor(CP_GREEN);
        else if (t.find("leaves") != std::string::npos)
            attr = uiColor(CP_CYAN);
        else if (t.find("arrives") != std::string::npos)
            attr = A_BOLD;

        // wrap at spaces; continuation lines are indented by two spaces
        std::string rest = t;
        bool firstLine = true;
        while (true)
        {
            size_t width = (size_t)std::max(1, textWidth - (firstLine ? 0 : 2));
            std::string prefix = firstLine ? "" : "  ";
            if (rest.size() <= width)
            {
                rowsOut.push_back({prefix + rest, attr});
                break;
            }
            size_t cut = rest.rfind(' ', width); // last space that still fits
            if (cut == std::string::npos || cut == 0)
                cut = width;
            rowsOut.push_back({prefix + rest.substr(0, cut), attr});
            rest = rest.substr(cut);
            while (!rest.empty() && rest[0] == ' ')
                rest.erase(0, 1);
            firstLine = false;
            if (rest.empty())
                break;
        }
    }
    size_t start = rowsOut.size() > (size_t)room ? rowsOut.size() - room : 0;
    // do not begin in the middle of an entry: skip its wrapped tail
    while (start < rowsOut.size() && start > 0 && rowsOut[start].text.compare(0, 2, "  ") == 0)
        start++;
    for (size_t i = start; i < rowsOut.size(); i++)
        text(rowsOut[i].text, rowsOut[i].attr);
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

    case 'm': case 'M': uiMono = !uiMono; break; // colour off/on

    case ' ': ui.paused = !ui.paused; break;
    case '+': case '=': ui.speedLevel = std::min(ui.speedLevel + 1, SPEED_COUNT - 1); break;
    case '-': case '_': ui.speedLevel = std::max(ui.speedLevel - 1, 0); break;

    case 'q': case 'Q': ui.quit = true; break;
    default: break;
    }
}
