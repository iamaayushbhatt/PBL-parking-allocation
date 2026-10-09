#include "intervaltree.h"
#include <algorithm>

IntervalTree::IntervalTree() : root(nullptr), count(0) {}

IntervalTree::~IntervalTree()
{
    destroy(root);
}

void IntervalTree::destroy(Node *n)
{
    if (n == nullptr)
        return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

// recompute maxHigh of n from its own high and its children
void IntervalTree::update(Node *n)
{
    n->maxHigh = n->high;
    if (n->left != nullptr)
        n->maxHigh = std::max(n->maxHigh, n->left->maxHigh);
    if (n->right != nullptr)
        n->maxHigh = std::max(n->maxHigh, n->right->maxHigh);
}

IntervalTree::Node *IntervalTree::insertNode(Node *n, int low, int high)
{
    if (n == nullptr)
        return new Node(low, high);

    if (low < n->low || (low == n->low && high < n->high))
        n->left = insertNode(n->left, low, high);
    else
        n->right = insertNode(n->right, low, high);

    update(n);
    return n;
}

IntervalTree::Node *IntervalTree::removeNode(Node *n, int low, int high, bool &removed)
{
    if (n == nullptr)
        return nullptr;

    if (low == n->low && high == n->high)
    {
        removed = true;

        if (n->left == nullptr)
        {
            Node *r = n->right;
            delete n;
            return r;
        }
        if (n->right == nullptr)
        {
            Node *l = n->left;
            delete n;
            return l;
        }

        // two children: copy the smallest node of the right subtree
        // into n, then delete that smallest node from the right subtree
        Node *m = n->right;
        while (m->left != nullptr)
            m = m->left;
        n->low = m->low;
        n->high = m->high;
        bool ignored = false;
        n->right = removeNode(n->right, m->low, m->high, ignored);
    }
    else if (low < n->low || (low == n->low && high < n->high))
    {
        n->left = removeNode(n->left, low, high, removed);
    }
    else
    {
        n->right = removeNode(n->right, low, high, removed);
    }

    update(n);
    return n;
}

bool IntervalTree::queryOverlap(int low, int high) const
{
    if (low >= high)
        return false;

    Node *n = root;
    while (n != nullptr)
    {
        // half-open overlap test
        if (n->low < high && low < n->high)
            return true;

        // go left only if something on the left ends after our start
        if (n->left != nullptr && n->left->maxHigh > low)
            n = n->left;
        else
            n = n->right;
    }
    return false;
}

bool IntervalTree::insert(int low, int high)
{
    if (low >= high)
        return false;
    root = insertNode(root, low, high);
    count++;
    return true;
}

bool IntervalTree::remove(int low, int high)
{
    bool removed = false;
    root = removeNode(root, low, high, removed);
    if (removed)
        count--;
    return removed;
}

bool IntervalTree::tryReserve(int low, int high)
{
    if (low >= high || queryOverlap(low, high))
        return false;
    return insert(low, high);
}

int IntervalTree::size() const
{
    return count;
}