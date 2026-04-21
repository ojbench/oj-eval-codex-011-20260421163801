#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include exceptions.hpp

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * **Exception Safety**: The  operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T val;
        Node *l, *r;
        int dist;
        explicit Node(const T &v) : val(v), l(nullptr), r(nullptr), dist(1) {}
    };

    Node *root = nullptr;
    size_t n = 0;
    Compare cmp;

    static int nd(Node *x) { return x ? x->dist : 0; }

    // Merge two leftist heaps and return new root. Strong exception safety:
    // structure updates occur only after potentially-throwing comparisons succeed.
    Node *merge_nodes(Node *a, Node *b) {
        if (!a) return b;
        if (!b) return a;
        bool use_b;
        try {
            // For max-heap semantics with Compare like std::less,
            // choose the larger element as the new root.
            use_b = cmp(a->val, b->val);
        } catch (...) {
            throw runtime_error();
        }
        Node *resRoot = a;
        Node *other = b;
        if (use_b) { resRoot = b; other = a; }
        // Recurse into the right child; only assign after successful return.
        Node *newRight = merge_nodes(resRoot->r, other);
        resRoot->r = newRight;
        // Maintain leftist property: dist(left) >= dist(right)
        if (nd(resRoot->l) < nd(resRoot->r)) {
            Node *tmp = resRoot->l; resRoot->l = resRoot->r; resRoot->r = tmp;
        }
        resRoot->dist = nd(resRoot->r) + 1;
        return resRoot;
    }

    static Node *clone(Node *x) {
        if (!x) return nullptr;
        Node *u = new Node(x->val);
        u->dist = x->dist;
        try {
            u->l = clone(x->l);
            u->r = clone(x->r);
        } catch (...) {
            clear(u);
            throw;
        }
        return u;
    }

    static void clear(Node *x) {
        if (!x) return;
        clear(x->l);
        clear(x->r);
        delete x;
    }
public:
    /**
     * @brief default constructor
     */
    priority_queue() = default;

    /**
     * @brief copy constructor
     * @param other the priority_queue to be copied
     */
    priority_queue(const priority_queue &other) : root(nullptr), n(other.n), cmp(other.cmp) {
        root = clone(other.root);
    }

    /**
     * @brief deconstructor
     */
    ~priority_queue() {
        clear(root);
        root = nullptr;
        n = 0;
    }

    /**
     * @brief Assignment operator
     * @param other the priority_queue to be assigned from
     * @return a reference to this priority_queue after assignment
     */
    priority_queue &operator=(const priority_queue &other) {
        if (this == &other) return *this;
        Node *newRoot = nullptr;
        try {
            newRoot = clone(other.root);
        } catch (...) {
            throw; // leave *this unchanged
        }
        clear(root);
        root = newRoot;
        n = other.n;
        cmp = other.cmp;
        return *this;
    }

    /**
     * @brief get the top element of the priority queue.
     * @return a reference of the top element.
     * @throws container_is_empty if empty() returns true
     */
    const T & top() const {
        if (!root) throw container_is_empty();
        return root->val;
    }

    /**
     * @brief push new element to the priority queue.
     * @param e the element to be pushed
     */
    void push(const T &e) {
        Node *node = nullptr;
        try {
            node = new Node(e);
            root = merge_nodes(root, node);
            ++n;
        } catch (...) {
            // If merge_nodes throws, node hasn't been linked (assignment happens after success)
            if (node && node->l == nullptr && node->r == nullptr && node != root) {
                delete node;
            }
            throw runtime_error();
        }
    }

    /**
     * @brief delete the top element from the priority queue.
     * @throws container_is_empty if empty() returns true
     */
    void pop() {
        if (!root) throw container_is_empty();
        Node *oldRoot = root;
        Node *L = root->l;
        Node *R = root->r;
        root = nullptr;
        try {
            Node *merged = merge_nodes(L, R);
            delete oldRoot;
            root = merged;
            --n;
        } catch (...) {
            // Restore original tree on comparator exception
            root = oldRoot;
            throw runtime_error();
        }
    }

    /**
     * @brief return the number of elements in the priority queue.
     * @return the number of elements.
     */
    size_t size() const { return n; }

    /**
     * @brief check if the container is empty.
     * @return true if it is empty, false otherwise.
     */
    bool empty() const { return n == 0; }

    /**
     * @brief merge another priority_queue into this one.
     * The other priority_queue will be cleared after merging.
     * The complexity is at most O(logn).
     * @param other the priority_queue to be merged.
     */
    void merge(priority_queue &other) {
        if (this == &other || !other.root) return;
        Node *a = root;
        Node *b = other.root;
        try {
            root = merge_nodes(a, b);
            n += other.n;
            other.root = nullptr;
            other.n = 0;
        } catch (...) {
            // Restore on failure
            root = a;
            throw runtime_error();
        }
    }
};

}

#endif
EOF}
