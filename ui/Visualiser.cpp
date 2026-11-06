// The curses screen: main loop, status line and the map.
#include "ui/Visualiser.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

static const int PANEL_WIDTH = 32;
static const int MIN_ROWS = 20;
static const int MIN_COLS = 84;

int uiColor(int pair) { return has_colors() ? COLOR_PAIR(pair) : 0; }

// starts curses in the constructor and always restores the terminal in the destructor
struct Screen
{
    Screen()
    {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
        if (has_colors())
        {
            start_color();
            init_pair(CP_GREEN, COLOR_GREEN, COLOR_BLACK);
            init_pair(CP_YELLOW, COLOR_YELLOW, COLOR_BLACK);
            init_pair(CP_RED, COLOR_RED, COLOR_BLACK);
            init_pair(CP_CYAN, COLOR_CYAN, COLOR_BLACK);
            init_pair(CP_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(CP_WHITE, COLOR_WHITE, COLOR_BLACK);
        }
    }
    ~Screen() { endwin(); }
};

// ------------------------------------------------------------ map geometry
// turns layout coordinates into screen cells
struct MapGeometry
{
    double minX = 0, minY = 0, sx = 1, sy = 0.5;
    int offX = 0, offY = 0;
    int cx(double x) const { return offX + (int)lround((x - minX) * sx); }
    int cy(double y) const { return offY + (int)lround((y - minY) * sy); }
};

static MapGeometry makeGeometry(const vector<NodeView> &nodes, int w, int h)
{
    MapGeometry g;
    if (nodes.empty())
        return g;

    double maxX = nodes[0].x, maxY = nodes[0].y;
    g.minX = nodes[0].x;
    g.minY = nodes[0].y;
    for (const NodeView &n : nodes)
    {
        g.minX = min(g.minX, n.x);
        g.minY = min(g.minY, n.y);
        maxX = max(maxX, n.x);
        maxY = max(maxY, n.y);
    }
    double rx = maxX - g.minX, ry = maxY - g.minY;

    // room left for the "IN" and "OUT" labels, the slot row and the legend
    const int left = 5, right = 8, top = 2, bottom = 5;
    double availW = max(1, w - left - right);
    double availH = max(1, h - top - bottom);

    // a terminal cell is about twice as tall as it is wide, so one unit
    // is s cells across but only s/2 cells down
    double s = 1e9;
    if (rx > 1e-9)
        s = min(s, availW / rx);
    if (ry > 1e-9)
        s = min(s, 2.0 * availH / ry);
    if (s > 1e8)
        s = 1;
    s = min(s, 6.0);

    g.sx = s;
    g.sy = s / 2;
    g.offX = left + (int)((availW - rx * g.sx) / 2);
    g.offY = top + (int)((availH - ry * g.sy) / 2);
    return g;
}

// ------------------------------------------------------------ drawing helpers
static void put(WINDOW *win, int y, int x, chtype ch, int attr = 0)
{
    int h, w;
    getmaxyx(win, h, w);
    if (y <= 0 || y >= h - 1 || x <= 0 || x >= w - 1)
        return; // never draw on the border or outside the window
    mvwaddch(win, y, x, ch | attr);
}

static void putText(WINDOW *win, int y, int x, const string &text, int attr = 0)
{
    for (size_t i = 0; i < text.size(); i++)
        put(win, y, x + (int)i, (unsigned char)text[i], attr);
}

// a straight line between two cells
static void drawLine(WINDOW *win, int x0, int y0, int x1, int y1, int attr, char forced = 0)
{
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    char ch = forced;
    if (!ch)
    {
        if (dy == 0)
            ch = '-';
        else if (dx == 0)
            ch = '|';
        else
            ch = ((x1 - x0) * (y1 - y0) > 0) ? '\\' : '/';
    }
    int sxStep = x0 < x1 ? 1 : -1, syStep = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int x = x0, y = y0;
    while (true)
    {
        put(win, y, x, ch, attr);
        if (x == x1 && y == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sxStep;
        }
        if (e2 < dx)
        {
            err += dx;
            y += syStep;
        }
    }
}

static int categoryColor(const string &category)
{
    if (category == "Electric")
        return uiColor(CP_CYAN);
    if (category == "Accessible")
        return uiColor(CP_MAGENTA);
    return uiColor(CP_WHITE);
}

static char categoryLetter(const string &category)
{
    if (category == "Electric")
        return 'E';
    if (category == "Accessible")
        return 'A';
    return 'C';
}

// ------------------------------------------------------------------ the map
static void drawMap(WINDOW *win, const vector<NodeView> &nodes, const SimSnapshot &snap)
{
    int h, w;
    getmaxyx(win, h, w);
    werase(win);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+'); // plain ASCII: works in every terminal

    MapGeometry g = makeGeometry(nodes, w, h);
    auto nodeX = [&](int id) { return g.cx(nodes[id].x); };
    auto nodeY = [&](int id) { return g.cy(nodes[id].y); };

    // 1. lanes: colour shows how full they are, red x means closed
    for (const LaneView &l : snap.lanes)
    {
        if (l.from < 0 || l.to < 0 || l.from >= (int)nodes.size() || l.to >= (int)nodes.size())
            continue;
        int attr = uiColor(CP_WHITE);
        char forced = 0;
        if (l.closed)
        {
            attr = uiColor(CP_RED) | A_BOLD;
            forced = 'x';
        }
        else if (l.capacity > 0 && l.vehicles >= l.capacity)
            attr = uiColor(CP_RED);
        else if (l.capacity > 0 && l.vehicles * 2 >= l.capacity)
            attr = uiColor(CP_YELLOW);
        else
            attr = uiColor(CP_GREEN);
        drawLine(win, nodeX(l.from), nodeY(l.from), nodeX(l.to), nodeY(l.to), attr, forced);
    }

    // 2. the route of every driving car, drawn over the lanes
    for (const CarView &c : snap.cars)
    {
        if (c.parked)
            continue;
        for (size_t i = 0; i + 1 < c.route.size(); i++)
        {
            int a = c.route[i], b = c.route[i + 1];
            if (a < 0 || b < 0 || a >= (int)nodes.size() || b >= (int)nodes.size())
                continue;
            drawLine(win, nodeX(a), nodeY(a), nodeX(b), nodeY(b), uiColor(CP_CYAN) | A_BOLD, '=');
        }
    }

    // 3. junctions and gates
    for (const NodeView &n : nodes)
    {
        int x = nodeX(n.id), y = nodeY(n.id);
        put(win, y, x, '+', A_BOLD);
        if (n.isEntry)
            putText(win, y, x - 3, "IN", uiColor(CP_GREEN) | A_BOLD);
        if (n.isExit)
            putText(win, y, x + 2, "OUT", uiColor(CP_RED) | A_BOLD);
    }

    // 4. parking slots: a row of one-character markers next to each junction
    map<string, const CarView *> parkedCars; // vehicle id -> car
    for (const CarView &c : snap.cars)
        if (c.parked)
            parkedCars[c.id] = &c;

    map<int, vector<const SlotView *>> slotsAtNode;
    for (const SlotView &s : snap.slots)
        slotsAtNode[s.node].push_back(&s);

    for (auto &entry : slotsAtNode)
    {
        int node = entry.first;
        if (node < 0 || node >= (int)nodes.size())
            continue;
        vector<const SlotView *> &list = entry.second;
        sort(list.begin(), list.end(), [](const SlotView *a, const SlotView *b) { return a->id < b->id; });

        int x0 = nodeX(node) - (int)list.size() / 2;
        int y = nodeY(node) + 1;
        if (y > h - 5)
            y = nodeY(node) - 1; // no room below: put the row above
        for (size_t i = 0; i < list.size(); i++)
        {
            const SlotView &s = *list[i];
            chtype ch;
            int attr;
            if (!s.occupied)
            {
                ch = s.ev ? 'e' : (s.accessible ? 'a' : 'o');
                attr = uiColor(CP_GREEN);
            }
            else if (s.occupant.empty())
            {
                ch = '~'; // reserved: a car is driving there
                attr = uiColor(CP_YELLOW) | A_BOLD;
            }
            else
            {
                auto car = parkedCars.find(s.occupant);
                string category = car != parkedCars.end() ? car->second->category : "Standard";
                ch = categoryLetter(category);
                attr = categoryColor(category) | A_REVERSE | A_BOLD;
            }
            put(win, y, x0 + (int)i, ch, attr);
        }
    }

    // 5. driving cars: their id, one row above the lane if the spot is taken
    set<pair<int, int>> used;
    for (const CarView &c : snap.cars)
    {
        if (c.parked)
            continue;
        int len = (int)c.id.size();
        int x = g.cx(c.x) - len / 2;
        int y = g.cy(c.y);
        for (int tries = 0; tries < 4; tries++)
        {
            bool free = true;
            for (int i = 0; i < len; i++)
                if (used.count({y, x + i}))
                    free = false;
            if (free)
                break;
            y--;
        }
        for (int i = 0; i < len; i++)
            used.insert({y, x + i});
        putText(win, y, x, c.id, categoryColor(c.category) | A_REVERSE | A_BOLD);
    }

    // 6. legend on the last row
    putText(win, h - 2, 2, "o free  e EV  a access  ~ reserved  C/E/A parked  = route  x closed");
}

// ------------------------------------------------------------ status and help
static void drawStatus(const Simulation &sim, const SimSnapshot &snap, const UiState &ui, int cols, int rows)
{
    char line[256];
    snprintf(line, sizeof line, " %s   |   t = %.1f min   |   speed x%g   |   %s   |   random traffic: %s",
             sim.layoutName().c_str(), snap.time, uiSpeed(ui), ui.paused ? "PAUSED" : "running",
             ui.randomOn ? "ON" : "off");
    attron(A_REVERSE);
    mvhline(0, 0, ' ', cols);
    mvaddnstr(0, 0, line, cols);
    attroff(A_REVERSE);

    attron(A_REVERSE);
    mvhline(rows - 1, 0, ' ', cols);
    mvaddnstr(rows - 1, 0,
              " s std  e EV  a accessible  l large  c compact  |  x car leaves  r random  space pause  + - speed  q quit",
              cols);
    attroff(A_REVERSE);
}

// ------------------------------------------------------------------ main loop
int runVisualiser(Simulation &sim)
{
    Screen screen;
    UiState ui;
    deque<string> log;
    const vector<NodeView> nodes = sim.nodes();

    WINDOW *mapWin = nullptr, *panelWin = nullptr;
    int rows = 0, cols = 0;
    bool firstFrame = true;

    double clock = sim.time();
    auto last = chrono::steady_clock::now();
    timeout(50); // getch waits at most 50 ms, so the screen keeps moving

    while (!ui.quit)
    {
        // (re)build the two windows if the terminal size changed
        int r, c;
        getmaxyx(stdscr, r, c);
        if (r != rows || c != cols || firstFrame)
        {
            firstFrame = false;
            rows = r;
            cols = c;
            if (mapWin)
                delwin(mapWin);
            if (panelWin)
                delwin(panelWin);
            mapWin = panelWin = nullptr;
            clear();
            if (rows >= MIN_ROWS && cols >= MIN_COLS)
            {
                mapWin = newwin(rows - 2, cols - PANEL_WIDTH, 1, 0);
                panelWin = newwin(rows - 2, PANEL_WIDTH, 1, cols - PANEL_WIDTH);
            }
        }

        // keys
        int key = getch();
        if (key == KEY_RESIZE)
        {
#ifdef PDCURSES
            resize_term(0, 0); // PDCurses needs to be told; ncurses does it by itself
#endif
        }
        else if (key != ERR)
            handleKey(key, sim, ui);

        // time
        auto now = chrono::steady_clock::now();
        double dt = chrono::duration<double>(now - last).count();
        last = now;
        if (!ui.paused)
            clock += dt * uiSpeed(ui);
        sim.advanceTo(clock); // also handles spawns and exits while paused

        for (const string &line : sim.takeLog())
        {
            log.push_back(line);
            if (log.size() > 300)
                log.pop_front();
        }
        SimSnapshot snap = sim.snapshot();

        // draw
        if (mapWin == nullptr)
        {
            clear();
            char msg[96];
            snprintf(msg, sizeof msg, "Terminal too small: need at least %dx%d, have %dx%d. Resize it, or press q.",
                     MIN_COLS, MIN_ROWS, cols, rows);
            mvaddnstr(0, 0, msg, cols);
            wnoutrefresh(stdscr);
        }
        else
        {
            drawStatus(sim, snap, ui, cols, rows);
            drawMap(mapWin, nodes, snap);
            drawPanel(panelWin, snap, log);
            wnoutrefresh(stdscr);
            wnoutrefresh(mapWin);
            wnoutrefresh(panelWin);
        }
        doupdate();
    }

    if (mapWin)
        delwin(mapWin);
    if (panelWin)
        delwin(panelWin);
    return 0;
}
