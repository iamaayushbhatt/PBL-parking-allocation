#include <cassert>
#include <iostream>
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

    std::cout << "All interval tree tests passed\n";
    return 0;
}