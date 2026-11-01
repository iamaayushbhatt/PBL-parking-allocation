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

int IntervalTree::heightOf(const Node *n)
{
    return n == nullptr ? 0 : n->height;
}

// recompute height and maxHigh of n from its own values and its children
void IntervalTree::update(Node *n)
{
    n->height = 1 + std::max(heightOf(n->left), heightOf(n->right));
    n->maxHigh = n->high;
    if (n->left != nullptr)
        n->maxHigh = std::max(n->maxHigh, n->left->maxHigh);
    if (n->right != nullptr)
        n->maxHigh = std::max(n->maxHigh, n->right->maxHigh);
}

// right rotation: the left child x of y moves up and y becomes x's right child;
// x's old right subtree becomes y's new left subtree
IntervalTree::Node *IntervalTree::rotateRight(Node *y)
{
    Node *x = y->left;
    y->left = x->right;
    x->right = y;
    update(y); // y is now lower, so fix it first
    update(x);
    return x;
}

// left rotation: the right child y of x moves up and x becomes y's left child;
// y's old left subtree becomes x's new right subtree
IntervalTree::Node *IntervalTree::rotateLeft(Node *x)
{
    Node *y = x->right;
    x->right = y->left;
    y->left = x;
    update(x);
    update(y);
    return y;
}

// update n, then rotate if one side is 2 or more levels taller than the other
IntervalTree::Node *IntervalTree::rebalance(Node *n)
{
    update(n);
    int balance = heightOf(n->left) - heightOf(n->right);

    if (balance > 1) // left side too tall
    {
        if (heightOf(n->left->left) < heightOf(n->left->right))
            n->left = rotateLeft(n->left); // left-right case
        return rotateRight(n);
    }
    if (balance < -1) // right side too tall
    {
        if (heightOf(n->right->right) < heightOf(n->right->left))
            n->right = rotateRight(n->right); // right-left case
        return rotateLeft(n);
    }
    return n;
}

IntervalTree::Node *IntervalTree::insertNode(Node *n, int low, int high)
{
    if (n == nullptr)
        return new Node(low, high);

    if (low < n->low || (low == n->low && high < n->high))
        n->left = insertNode(n->left, low, high);
    else
        n->right = insertNode(n->right, low, high);

    return rebalance(n);
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

    return rebalance(n);
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

int IntervalTree::height() const
{
    return heightOf(root);
}