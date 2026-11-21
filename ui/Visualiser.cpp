// The curses screen: main loop, status bar, key bar and the map.
#include "ui/Visualiser.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

static const int PANEL_WIDTH = 34;
static const int MIN_ROWS = 26;
static const int MIN_COLS = 100;

bool uiMono = false;
bool uiHasColor() { return has_colors() && !uiMono; }
chtype uiColor(int pair) { return uiHasColor() ? COLOR_PAIR(pair) : 0; }
chtype uiBlock(int pair) { return uiHasColor() ? COLOR_PAIR(pair) : A_REVERSE; }

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
            init_pair(CP_ROAD, COLOR_BLACK, COLOR_WHITE);
            init_pair(CP_ROAD_BUSY, COLOR_BLACK, COLOR_YELLOW);
            init_pair(CP_ROAD_FULL, COLOR_WHITE, COLOR_RED);
            init_pair(CP_ROUTE, COLOR_WHITE, COLOR_BLUE);
            init_pair(CP_CAR_STD, COLOR_BLACK, COLOR_GREEN);
            init_pair(CP_CAR_EV, COLOR_BLACK, COLOR_CYAN);
            init_pair(CP_CAR_ACC, COLOR_WHITE, COLOR_MAGENTA);
            init_pair(CP_BAR, COLOR_BLACK, COLOR_WHITE);
            init_pair(CP_KEY, COLOR_BLACK, COLOR_CYAN);
        }
    }
    ~Screen() { endwin(); }
};

// ------------------------------------------------------------ text segments
// a piece of text with its own attribute; a row is a list of them
struct Seg
{
    string text;
    chtype attr;
};

static Seg mk(const string &text, chtype attr)
{
    Seg s;
    s.text = text;
    s.attr = attr;
    return s;
}

static void drawSegs(WINDOW *win, int y, int x, const vector<Seg> &segs, int limitX)
{
    for (const Seg &s : segs)
    {
        if (x >= limitX)
            break;
        wattron(win, s.attr);
        mvwaddnstr(win, y, x, s.text.c_str(), limitX - x);
        wattroff(win, s.attr);
        x += (int)s.text.size();
    }
}

static int segsWidth(const vector<Seg> &segs)
{
    int n = 0;
    for (const Seg &s : segs)
        n += (int)s.text.size();
    return n;
}

// ------------------------------------------------------------ map geometry
// turns layout coordinates into screen cells
struct MapGeometry
{
    double minX = 0, minY = 0, sx = 1, sy = 0.5;
    int offX = 0, offY = 0;
    int cx(double x) const { return offX + (int)lround((x - minX) * sx); }
    int cy(double y) const { return offY + (int)lround((y - minY) * sy); }
};

// legendRows = how many rows the legend needs at the bottom of the map
static MapGeometry makeGeometry(const vector<NodeView> &nodes, int w, int h, int legendRows)
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

    // room left for the "IN>" and "OUT>" labels, a slot row under the last junction and the legend
    const int left = 7, right = 10, top = 3;
    const int bottom = legendRows + 4;
    double availW = max(1, w - left - right);
    double availH = max(1, h - top - bottom);

    // The two directions are scaled separately, so a tall lot still uses the width of the window.
    // Sideways one unit is sx cells; down it is sy rows, never more than sx (a row is about twice
    // as tall as a column is wide, so sy = sx / 2 would be square on screen).
    double sx = 1e9;
    if (rx > 1e-9)
        sx = availW / rx;
    if (sx > 1e8)
        sx = 1;
    sx = min(sx, 8.0);
    double sy = 0.5 * sx;
    if (ry > 1e-9)
        sy = min(availH / ry, sx);

    g.sx = sx;
    g.sy = sy;
    g.offX = left + (int)((availW - rx * g.sx) / 2);
    g.offY = top + (int)((availH - ry * g.sy) / 2);
    return g;
}

// ------------------------------------------------------------ drawing helpers
static void put(WINDOW *win, int y, int x, chtype ch, chtype attr = 0)
{
    int h, w;
    getmaxyx(win, h, w);
    if (y <= 0 || y >= h - 1 || x <= 0 || x >= w - 1)
        return; // never draw on the border or outside the window
    mvwaddch(win, y, x, ch | attr);
}

static void putText(WINDOW *win, int y, int x, const string &text, chtype attr = 0)
{
    for (size_t i = 0; i < text.size(); i++)
        put(win, y, x + (int)i, (unsigned char)text[i], attr);
}

// every cell on the straight line between two cells, end points included
static vector<pair<int, int>> lineCells(int x0, int y0, int x1, int y1)
{
    vector<pair<int, int>> cells;
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sxStep = x0 < x1 ? 1 : -1, syStep = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    int x = x0, y = y0;
    while (true)
    {
        cells.push_back({x, y});
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
    return cells;
}

// arrow for a direction given in screen cells (a row is about twice as tall as a column is wide)
static char arrowFor(int dx, int dy)
{
    if (abs(dx) >= 2 * abs(dy))
        return dx >= 0 ? '>' : '<';
    return dy >= 0 ? 'v' : '^';
}

static int carPair(const string &category)
{
    if (category == "Electric")
        return CP_CAR_EV;
    if (category == "Accessible")
        return CP_CAR_ACC;
    return CP_CAR_STD;
}

// shortens an id to fit `width` characters (keeps the number at the end) and centres it
static string fitLabel(const string &id, int width)
{
    string t = (int)id.size() > width ? id.substr(id.size() - width) : id;
    int pad = width - (int)t.size();
    return string(pad / 2, ' ') + t + string(pad - pad / 2, ' ');
}

// ------------------------------------------------------------------- legend
// rows of sample + label pairs; each pair stays together when a row has to wrap
typedef vector<Seg> Item;

static vector<vector<Seg>> buildLegend(int width)
{
    bool col = uiHasColor();
    auto sample = [&](const char *colourText, const char *plainText, int pair) {
        return col ? mk(colourText, uiBlock(pair)) : mk(plainText, 0);
    };

    vector<vector<Item>> rows(3);

    Item head;
    head.push_back(mk("Roads ", A_BOLD));
    rows[0].push_back(head);
    {
        Item i;
        i.push_back(sample("   ", "...", CP_ROAD));
        i.push_back(mk(" clear  ", 0));
        rows[0].push_back(i);
    }
    {
        Item i;
        i.push_back(sample("   ", ":::", CP_ROAD_BUSY));
        i.push_back(mk(" busy  ", 0));
        rows[0].push_back(i);
    }
    {
        Item i;
        i.push_back(sample("   ", "###", CP_ROAD_FULL));
        i.push_back(mk(" full  ", 0));
        rows[0].push_back(i);
    }
    {
        Item i;
        i.push_back(mk("X X", uiColor(CP_RED) | A_BOLD));
        i.push_back(mk(" closed  ", 0));
        rows[0].push_back(i);
    }
    {
        Item i;
        i.push_back(sample(">>>", ">>>", CP_ROUTE));
        i.push_back(mk(" route", 0));
        rows[0].push_back(i);
    }

    Item head2;
    head2.push_back(mk("Slots ", A_BOLD));
    rows[1].push_back(head2);
    {
        Item i;
        i.push_back(mk("[ 1 ]", uiColor(CP_GREEN) | A_BOLD));
        i.push_back(mk(" free  ", 0));
        rows[1].push_back(i);
    }
    {
        Item i;
        i.push_back(mk("[ 5 ]", uiColor(CP_CYAN) | A_BOLD));
        i.push_back(mk(" EV  ", 0));
        rows[1].push_back(i);
    }
    {
        Item i;
        i.push_back(mk("[ 11]", uiColor(CP_MAGENTA) | A_BOLD));
        i.push_back(mk(" accessible  ", 0));
        rows[1].push_back(i);
    }
    {
        Item i;
        i.push_back(mk("[V4 ]", uiColor(CP_YELLOW) | A_BOLD));
        i.push_back(mk(" reserved  ", 0));
        rows[1].push_back(i);
    }
    {
        Item i;
        i.push_back(mk("( ) compact  [ ] standard  { } large", 0));
        rows[1].push_back(i);
    }

    Item head3;
    head3.push_back(mk("Cars  ", A_BOLD));
    rows[2].push_back(head3);
    {
        Item i;
        i.push_back(sample(" V1>", " V1>", CP_CAR_STD));
        i.push_back(mk(" standard  ", 0));
        rows[2].push_back(i);
    }
    {
        Item i;
        i.push_back(sample(" V2>", " V2>", CP_CAR_EV));
        i.push_back(mk(" electric  ", 0));
        rows[2].push_back(i);
    }
    {
        Item i;
        i.push_back(sample(" V3>", " V3>", CP_CAR_ACC));
        i.push_back(mk(" accessible  ", 0));
        rows[2].push_back(i);
    }
    {
        Item i;
        i.push_back(mk("arrow = heading, filled slot = parked car", 0));
        rows[2].push_back(i);
    }

    // wrap each row greedily, the heading goes on the first line only
    vector<vector<Seg>> lines;
    for (const vector<Item> &row : rows)
    {
        vector<Seg> cur;
        int used = 0;
        for (size_t k = 0; k < row.size(); k++)
        {
            int len = segsWidth(row[k]);
            if (used + len > width && used > 6)
            {
                lines.push_back(cur);
                cur.clear();
                cur.push_back(mk("      ", 0)); // indent under the heading
                used = 6;
            }
            for (const Seg &s : row[k])
                cur.push_back(s);
            used += len;
        }
        if (!cur.empty())
            lines.push_back(cur);
    }
    return lines;
}

// ------------------------------------------------------------------ the map
static void drawMap(WINDOW *win, const vector<NodeView> &nodes, const SimSnapshot &snap)
{
    int h, w;
    getmaxyx(win, h, w);
    werase(win);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+'); // plain ASCII: works in every terminal
    mvwaddstr(win, 0, 2, " PARKING LOT ");

    vector<vector<Seg>> legend = buildLegend(w - 4);
    int L = (int)legend.size();

    MapGeometry g = makeGeometry(nodes, w, h, L);
    auto nodeX = [&](int id) { return g.cx(nodes[id].x); };
    auto nodeY = [&](int id) { return g.cy(nodes[id].y); };
    bool col = uiHasColor();

    set<pair<int, int>> roadCells; // (row, column) taken by a road
    set<pair<int, int>> routeCells;

    // a road cell is two columns wide so a vertical road looks as thick as a horizontal one
    auto roadCell = [&](int x, int y, chtype attr, char glyph) {
        for (int k = 0; k < 2; k++)
        {
            put(win, y, x + k, glyph, attr);
            roadCells.insert({y, x + k});
        }
    };

    // 1. lanes. One state per road, the worse of its two directions:
    //    0 clear, 1 half full, 2 full, 3 closed
    map<pair<int, int>, int> laneState;
    for (const LaneView &l : snap.lanes)
    {
        if (l.from < 0 || l.to < 0 || l.from >= (int)nodes.size() || l.to >= (int)nodes.size())
            continue;
        int st = 0;
        if (l.closed)
            st = 3;
        else if (l.capacity > 0 && l.vehicles >= l.capacity)
            st = 2;
        else if (l.capacity > 0 && l.vehicles * 2 >= l.capacity)
            st = 1;
        pair<int, int> key = {min(l.from, l.to), max(l.from, l.to)};
        laneState[key] = max(laneState[key], st);
    }
    for (const auto &entry : laneState)
    {
        int a = entry.first.first, b = entry.first.second, st = entry.second;
        for (const auto &c : lineCells(nodeX(a), nodeY(a), nodeX(b), nodeY(b)))
        {
            chtype attr;
            char glyph;
            if (st == 3)
            {
                attr = uiColor(CP_RED) | A_BOLD;
                glyph = ((c.first + c.second) % 2 == 0) ? 'X' : ' ';
            }
            else
            {
                static const int pairs[] = {CP_ROAD, CP_ROAD_BUSY, CP_ROAD_FULL};
                static const char plain[] = {'.', ':', '#'};
                attr = uiBlock(pairs[st]);
                glyph = col ? ' ' : plain[st];
            }
            roadCell(c.first, c.second, attr, glyph);
        }
    }

    // 2. what every driving car does next: the part of its route it has not driven yet,
    //    drawn over the roads with arrows pointing the way
    map<string, char> carArrow;
    for (const CarView &c : snap.cars)
    {
        if (c.parked || c.route.size() < 2)
            continue;
        int px = g.cx(c.x), py = g.cy(c.y);

        // the route segment the car is on is the one it is closest to
        size_t cur = 0;
        double best = 1e18;
        for (size_t i = 0; i + 1 < c.route.size(); i++)
        {
            int a = c.route[i], b = c.route[i + 1];
            if (a < 0 || b < 0 || a >= (int)nodes.size() || b >= (int)nodes.size())
                continue;
            double ax = nodeX(a), ay = nodeY(a) * 2.0, bx = nodeX(b), by = nodeY(b) * 2.0;
            double vx = bx - ax, vy = by - ay, len2 = vx * vx + vy * vy;
            double t = len2 > 0 ? ((px - ax) * vx + (py * 2.0 - ay) * vy) / len2 : 0;
            t = max(0.0, min(1.0, t));
            double dx = ax + t * vx - px, dy = ay + t * vy - py * 2.0;
            double d = dx * dx + dy * dy;
            if (d < best)
            {
                best = d;
                cur = i;
            }
        }

        for (size_t i = cur; i + 1 < c.route.size(); i++)
        {
            int a = c.route[i], b = c.route[i + 1];
            if (a < 0 || b < 0 || a >= (int)nodes.size() || b >= (int)nodes.size())
                continue;
            int x0 = nodeX(a), y0 = nodeY(a), x1 = nodeX(b), y1 = nodeY(b);
            char arrow = arrowFor(x1 - x0, y1 - y0);
            if (i == cur)
                carArrow[c.id] = arrow;
            vector<pair<int, int>> cells = lineCells(x0, y0, x1, y1);
            for (size_t k = 0; k < cells.size(); k++)
            {
                char glyph = (k % 3 == 1) ? arrow : (col ? ' ' : '=');
                for (int m = 0; m < 2; m++)
                {
                    put(win, cells[k].second, cells[k].first + m, (m == 0 ? glyph : ' '), uiBlock(CP_ROUTE));
                    routeCells.insert({cells[k].second, cells[k].first + m});
                }
            }
        }
    }

    // 3. junctions and gates
    for (const NodeView &n : nodes)
    {
        int x = nodeX(n.id), y = nodeY(n.id);
        chtype attr = routeCells.count({y, x}) ? uiBlock(CP_ROUTE) : uiBlock(CP_ROAD);
        put(win, y, x, '+', attr);
        put(win, y, x + 1, '+', attr);
        if (n.isEntry)
            putText(win, y, x - 5, " IN> ", uiColor(CP_GREEN) | A_BOLD | (col ? 0 : A_REVERSE));
        if (n.isExit)
            putText(win, y, x + 3, " OUT>", uiColor(CP_RED) | A_BOLD | (col ? 0 : A_REVERSE));
    }

    // 4. parking slots: a row of small boxes next to each junction, each showing the slot
    //    number, or the car that is parked there. ( ) compact, [ ] standard, { } large.
    map<string, const CarView *> carById;
    map<int, const CarView *> drivingTo; // slot id -> car that has it reserved
    for (const CarView &c : snap.cars)
    {
        carById[c.id] = &c;
        if (!c.parked)
            drivingTo[c.slotId] = &c;
    }

    struct Group
    {
        int node, cx, cy, iw;
        vector<const SlotView *> slots;
    };
    map<int, Group> groups;
    for (const SlotView &s : snap.slots)
    {
        if (s.node < 0 || s.node >= (int)nodes.size())
            continue;
        Group &gr = groups[s.node];
        gr.node = s.node;
        gr.cx = nodeX(s.node);
        gr.cy = nodeY(s.node);
        gr.slots.push_back(&s);
    }
    for (auto &entry : groups)
    {
        Group &gr = entry.second;
        sort(gr.slots.begin(), gr.slots.end(), [](const SlotView *a, const SlotView *b) { return a->id < b->id; });
        // how much sideways room does this group have before the next group's row?
        int avail = 1000;
        for (const auto &other : groups)
            if (other.first != entry.first && abs(other.second.cy - gr.cy) <= 3)
                avail = min(avail, abs(other.second.cx - gr.cx) - 1);
        int n = (int)gr.slots.size();
        gr.iw = 1;
        for (int iw = 3; iw >= 1; iw--)
            if (n * (iw + 2) <= avail)
            {
                gr.iw = iw;
                break;
            }
    }

    set<pair<int, int>> slotCells;
    int lowestRow = h - L - 3; // keep one blank row above the legend
    for (auto &entry : groups)
    {
        Group &gr = entry.second;
        int n = (int)gr.slots.size();
        int gw = n * (gr.iw + 2);
        int centre = gr.cx + 1 - gw / 2;

        // first spot that is inside the window and does not sit on a road or another group
        struct Spot
        {
            int y, x;
        };
        Spot spots[] = {{gr.cy + 1, centre},        {gr.cy - 1, centre},
                        {gr.cy + 1, gr.cx + 3},     {gr.cy + 1, gr.cx - 2 - gw},
                        {gr.cy - 1, gr.cx + 3},     {gr.cy - 1, gr.cx - 2 - gw},
                        {gr.cy + 2, centre},        {gr.cy - 2, centre}};
        Spot pick = spots[0];
        for (const Spot &sp : spots)
        {
            bool ok = sp.y >= 1 && sp.y <= lowestRow && sp.x >= 1 && sp.x + gw <= w - 1;
            for (int i = 0; ok && i < gw; i++)
                if (roadCells.count({sp.y, sp.x + i}) || slotCells.count({sp.y, sp.x + i}))
                    ok = false;
            if (ok)
            {
                pick = sp;
                break;
            }
        }

        for (int i = 0; i < n; i++)
        {
            const SlotView &s = *gr.slots[i];
            char open = '[', close = ']';
            if (s.size == "COMPACT")
                open = '(', close = ')';
            else if (s.size == "LARGE")
                open = '{', close = '}';

            string label;
            chtype attr;
            if (!s.occupied)
            {
                label = to_string(s.id);
                attr = (s.ev ? uiColor(CP_CYAN) : s.accessible ? uiColor(CP_MAGENTA) : uiColor(CP_GREEN)) | A_BOLD;
            }
            else if (s.occupant.empty())
            {
                // reserved: show the car that is driving there
                auto car = drivingTo.find(s.id);
                label = car != drivingTo.end() ? car->second->id : "~";
                attr = uiColor(CP_YELLOW) | A_BOLD;
            }
            else
            {
                auto car = carById.find(s.occupant);
                string category = car != carById.end() ? car->second->category : "Standard";
                label = s.occupant;
                attr = uiBlock(carPair(category));
            }
            string cell = string(1, open) + fitLabel(label, gr.iw) + string(1, close);
            putText(win, pick.y, pick.x + i * (gr.iw + 2), cell, attr);
            for (int k = 0; k < gr.iw + 2; k++)
                slotCells.insert({pick.y, pick.x + i * (gr.iw + 2) + k});
        }
    }

    // 5. driving cars: a coloured block with the id and an arrow for the heading
    set<pair<int, int>> used;
    for (const CarView &c : snap.cars)
    {
        if (c.parked)
            continue;
        auto ar = carArrow.find(c.id);
        string label = " " + c.id + (ar != carArrow.end() ? string(1, ar->second) : string(" "));
        int len = (int)label.size();
        int x = g.cx(c.x) + 1 - len / 2;
        int y = g.cy(c.y);
        int tryY[] = {y, y - 1, y + 1, y - 2, y + 2};
        for (int ty : tryY)
        {
            bool free = true;
            for (int i = 0; i < len; i++)
                if (used.count({ty, x + i}))
                    free = false;
            if (free)
            {
                y = ty;
                break;
            }
        }
        for (int i = 0; i < len; i++)
            used.insert({y, x + i});
        putText(win, y, x, label, uiBlock(carPair(c.category)));
    }

    // 6. legend at the bottom
    int y0 = h - 1 - L;
    for (int i = 0; i < L; i++)
        drawSegs(win, y0 + i, 2, legend[i], w - 1);
}

// ------------------------------------------------------------ status and key bars
static void addKey(vector<Seg> &row, const char *key, const char *label, chtype bar)
{
    string cap = uiHasColor() ? string(" ") + key + " " : string("[") + key + "]";
    row.push_back(mk(cap, uiBlock(CP_KEY)));
    row.push_back(mk(string(" ") + label + "  ", bar));
}

static void drawStatus(const Simulation &sim, const SimSnapshot &snap, const UiState &ui, int cols, int rows)
{
    chtype bar = uiBlock(CP_BAR);
    // paint both bars with spaces first (works the same on ncurses and PDCurses)
    vector<Seg> blank;
    blank.push_back(mk(string(cols, ' '), bar));
    drawSegs(stdscr, 0, 0, blank, cols);
    drawSegs(stdscr, rows - 1, 0, blank, cols);

    // top bar: layout, time, speed, state
    char t[48], sp[32];
    snprintf(t, sizeof t, " t = %.1f min ", snap.time);
    snprintf(sp, sizeof sp, " speed x%g ", uiSpeed(ui));
    vector<Seg> top;
    top.push_back(mk(" " + sim.layoutName() + " ", bar));
    top.push_back(mk("|", bar));
    top.push_back(mk(t, bar));
    top.push_back(mk("|", bar));
    top.push_back(mk(sp, bar));
    top.push_back(mk(" ", bar));
    if (ui.paused)
        top.push_back(mk(" PAUSED ", uiBlock(CP_ROAD_BUSY)));
    else
        top.push_back(mk(" RUNNING ", uiBlock(CP_CAR_STD)));
    top.push_back(mk(" ", bar));
    if (ui.randomOn)
        top.push_back(mk(" random ON ", uiBlock(CP_CAR_STD)));
    else
        top.push_back(mk(" random off ", bar));
    drawSegs(stdscr, 0, 0, top, cols);

    // bottom bar: the keys, as key caps. A long version if it fits, else a short one.
    vector<Seg> full;
    addKey(full, "s", "standard", bar);
    addKey(full, "e", "EV", bar);
    addKey(full, "a", "accessible", bar);
    addKey(full, "l", "large", bar);
    addKey(full, "c", "compact", bar);
    full.push_back(mk("|  ", bar));
    addKey(full, "x", "car leaves", bar);
    addKey(full, "r", "random", bar);
    addKey(full, "m", "mono", bar);
    addKey(full, "space", "pause", bar);
    addKey(full, "+/-", "speed", bar);
    addKey(full, "q", "quit", bar);

    vector<Seg> shortRow;
    addKey(shortRow, "s", "", bar);
    addKey(shortRow, "e", "", bar);
    addKey(shortRow, "a", "", bar);
    addKey(shortRow, "l", "", bar);
    addKey(shortRow, "c", "spawn car", bar);
    addKey(shortRow, "x", "leave", bar);
    addKey(shortRow, "r", "random", bar);
    addKey(shortRow, "m", "mono", bar);
    addKey(shortRow, "space", "pause", bar);
    addKey(shortRow, "+/-", "speed", bar);
    addKey(shortRow, "q", "quit", bar);

    drawSegs(stdscr, rows - 1, 0, segsWidth(full) + 1 <= cols ? full : shortRow, cols);
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
                // keep the last column free: some windows (PDCurses on Windows) hide it,
                // which cut off the panel's right border
                int usable = cols - 1;
                int panelW = (usable >= 160) ? 46 : PANEL_WIDTH; // wider log on big windows
                mapWin = newwin(rows - 2, usable - panelW, 1, 0);
                panelWin = newwin(rows - 2, panelW, 1, usable - panelW);
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
