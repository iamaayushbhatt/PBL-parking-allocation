// Entry point.
//   parking_sim [layout.json] [--random N] [--seed S]     text mode
//   parking_sim [layout.json] --ui                         screen (needs a build with -DWITH_UI)
//
// Text build (from the repo root):
//   g++ -std=c++17 -I. main.cpp Simulation.cpp scenario/RandomScenario.cpp graph.cpp pathfinding.cpp intervaltree.cpp models/Slot_State.cpp models/VehicleHierarchy.cpp models/Ticket_Metrics.cpp metrics/metrics.cpp visualization/JsonParser.cpp visualization/layoutLoader.cpp -o parking_sim
// Screen build: the same line plus  -DWITH_UI ui/Visualiser.cpp ui/Panel.cpp -lncurses
//   (on Windows with PDCurses the library is usually -lpdcurses)
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include "Simulation.h"
#ifdef WITH_UI
#include "ui/Visualiser.h"
#endif

using namespace std;

int main(int argc, char **argv)
{
    string path = "layouts/DemoLayout.json";
    bool random = false, ui = false;
    int count = 20;
    unsigned seed = 1;
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "--random" && i + 1 < argc)
        {
            random = true;
            count = atoi(argv[++i]);
        }
        else if (arg == "--seed" && i + 1 < argc)
            seed = (unsigned)atoi(argv[++i]);
        else if (arg == "--ui")
            ui = true;
        else
            path = arg;
    }
    try
    {
        if (ui)
        {
#ifdef WITH_UI
            Simulation sim(path, Simulation::Mode::Interactive);
            return runVisualiser(sim);
#else
            cerr << "This build has no screen. Rebuild with -DWITH_UI (see the top of main.cpp).\n";
            return 1;
#endif
        }
        Simulation sim(path, random ? Simulation::Mode::Random : Simulation::Mode::Demo, count, seed);
        sim.run();
    }
    catch (const exception &ex)
    {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
