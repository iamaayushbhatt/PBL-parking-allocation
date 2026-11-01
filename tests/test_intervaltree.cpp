#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "intervaltree.h"

int main()
{
    IntervalTree t;

    assert(!t.queryOverlap(0, 100));       // empty tree

    assert(t.insert(10, 20));

    assert(!t.queryOverlap(20, 30));       // adjacent: no conflict
    assert(!t.queryOverlap(0, 10));

    assert(t.queryOverlap(15, 25));        // partial overlap
    assert(t.queryOverlap(5, 12));

    assert(t.queryOverlap(12, 18));        // nested
    assert(t.queryOverlap(5, 25));         // contains it
    assert(t.queryOverlap(10, 20));        // identical

    assert(t.tryReserve(20, 30));
    assert(!t.tryReserve(25, 35));         // conflicts with [20,30)
    assert(t.tryReserve(40, 50));
    assert(t.size() == 3);

    assert(!t.insert(30, 30));             // invalid intervals
    assert(!t.insert(30, 20));
    assert(!t.tryReserve(60, 55));

    assert(t.queryOverlap(22, 24));
    assert(t.remove(20, 30));              // removing frees the window
    assert(!t.queryOverlap(22, 24));
    assert(!t.remove(20, 30));             // already gone
    assert(t.size() == 2);

    assert(t.tryReserve(20, 40));          // fills the gap exactly
    assert(t.size() == 3);
    assert(!t.tryReserve(0, 100));

    // --- balancing: bookings that arrive in sorted order must not make a line ---
    {
        IntervalTree sorted;
        for (int i = 0; i < 1000; i++)
            assert(sorted.tryReserve(i * 10, i * 10 + 10)); // back-to-back windows
        assert(sorted.size() == 1000);
        assert(sorted.height() <= 14); // an unbalanced tree would be 1000 levels tall

        assert(sorted.queryOverlap(4995, 5005)); // finds a window in the middle
        assert(!sorted.tryReserve(2500, 2501));  // already booked
        assert(sorted.remove(2500, 2510));
        assert(sorted.tryReserve(2500, 2510));   // window is free again
        assert(sorted.height() <= 14);
    }

    // --- random test: compare with a simple list of booked windows ---
    {
        IntervalTree tree;
        std::vector<std::pair<int, int>> booked;
        srand(12345);
        for (int step = 0; step < 5000; step++)
        {
            int low = rand() % 1000;
            int high = low + 1 + rand() % 50;

            bool clash = false;
            for (const auto &b : booked)
                if (b.first < high && low < b.second)
                    clash = true;

            assert(tree.queryOverlap(low, high) == clash);

            if (rand() % 3 != 0)
            {
                assert(tree.tryReserve(low, high) == !clash);
                if (!clash)
                    booked.push_back({low, high});
            }
            else if (!booked.empty())
            {
                int k = rand() % booked.size();
                assert(tree.remove(booked[k].first, booked[k].second));
                booked.erase(booked.begin() + k);
            }
            assert(tree.size() == (int)booked.size());
        }
        assert(tree.height() <= 20);
    }

    std::cout << "All interval tree tests passed\n";
    return 0;
}