#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include exceptions.hpp

namespace sjtu {
/**
 * a container like std::priority_queue which is a heap internal.
 * Exception Safety: The Compare operation might throw exceptions.
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

    Node *merge_nodes(Node *a, Node *b) {
        if (!a) return b;
        if (!b) return a;
        bool use_b;
        try {
            use_b = cmp(a->val, b->val);
        } catch (...) {
            throw runtime_error();
        }
        Node *resRoot = a;
        Node *other = b;
        if (use_b) { resRoot = b; other = a; }
        Node *newRight = merge_nodes(resRoot->r, other);
        resRoot->r = newRight;
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
    priority_queue() = default;

    priority_queue(const priority_queue &other) : root(nullptr), n(other.n), cmp(other.cmp) {
        root = clone(other.root);
    }

    ~priority_queue() {
        clear(root);
        root = nullptr;
        n = 0;
    }

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

    const T & top() const {
        if (!root) throw container_is_empty();
        return root->val;
    }

    void push(const T &e) {
        Node *node = nullptr;
        try {
            node = new Node(e);
            root = merge_nodes(root, node);
            ++n;
        } catch (...) {
            if (node && node->l == nullptr && node->r == nullptr && node != root) {
                delete node;
            }
            throw runtime_error();
        }
    }

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
            root = oldRoot;
            throw runtime_error();
        }
    }

    size_t size() const { return n; }

    bool empty() const { return n == 0; }

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
            root = a;
            throw runtime_error();
        }
    }
};

}

#endif
