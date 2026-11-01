#ifndef INTERVALTREE_H
#define INTERVALTREE_H

class IntervalTree
{
private:
    struct Node
    {
        int low;
        int high;
        int maxHigh;
        int height; // height of the subtree rooted here (a leaf has height 1)
        Node *left;
        Node *right;

        Node(int l, int h) : low(l), high(h), maxHigh(h), height(1), left(nullptr), right(nullptr) {}
    };

    Node *root;
    int count;

    static int heightOf(const Node *n);
    static void update(Node *n);
    static Node *rotateRight(Node *y);
    static Node *rotateLeft(Node *x);
    static Node *rebalance(Node *n);
    static Node *insertNode(Node *n, int low, int high);
    static Node *removeNode(Node *n, int low, int high, bool &removed);
    static void destroy(Node *n);

public:
    IntervalTree();
    ~IntervalTree();

    IntervalTree(const IntervalTree &) = delete;
    IntervalTree &operator=(const IntervalTree &) = delete;

    bool queryOverlap(int low, int high) const;
    bool insert(int low, int high);
    bool remove(int low, int high);
    bool tryReserve(int low, int high);
    int size() const;
    int height() const; // height of the whole tree (0 if empty)
};

#endif