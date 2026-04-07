#ifndef THREADED_AVL_H
#define THREADED_AVL_H

#include "AVL.h"

#include <list>

// =======================
// ThreadedAVL (SKELETON - BONUS)
// =======================
// TODO (bonus):
// - Add inorder threading (prev/next) to support O(1) next/previous navigation.
// - ThreadedAVL inherits AVL and uses hooks to maintain threads.
// - Rotation does NOT change inorder order, so threading is updated mainly in insert/erase hooks.

template <typename K, typename V>
class ThreadedAVL : public AVL<K, V> {
private:
    using Base = AVL<K, V>;
    using AVLNode = typename Base::Node;

    struct TNode : public AVLNode {
        TNode* prev;
        TNode* next;

        TNode(const K& k, const V& v)
            : AVLNode(k, v), prev(nullptr), next(nullptr) {}
    };

    TNode* head_; // smallest
    TNode* tail_; // largest

public:
    class Iterator {
        friend class ThreadedAVL<K, V>;
        TNode* p_;
        explicit Iterator(TNode* p) : p_(p) {}
    public:
        Iterator() : p_(nullptr) {}

        bool operator==(const Iterator& other) const { return p_ == other.p_; }
        bool operator!=(const Iterator& other) const { return p_ != other.p_; }

        // TODO: O(1) move using next/prev
        Iterator& operator++() { if (p_) p_ = p_->next; return *this; }
        Iterator& operator--() { if (p_) p_ = p_->prev; return *this; }

        const K& key() const { return p_->key; }
        V& value() { return p_->value; }
        const V& value() const { return p_->value; }
        bool isNull() const { return p_ == nullptr; }
    };

public:
    ThreadedAVL() : Base(), head_(nullptr), tail_(nullptr) {}
    ~ThreadedAVL() override { this->clear(); }

    // TODO: These should traverse via threading (O(n) but very fast in practice)
    std::list<K> ascendingList() const override {
        // TODO
        return {};
    }

    std::list<K> descendingList() const override {
        // TODO
        return {};
    }

    void clear() override {
        // TODO: call Base::clear() + reset head_/tail_
        Base::clear();
        head_ = tail_ = nullptr;
    }

    // Iteration helpers
    Iterator beginIt() { return Iterator(head_); }
    Iterator endIt() { return Iterator(nullptr); }
    Iterator rbeginIt() { return Iterator(tail_); }

    Iterator findIt(const K& /*key*/) {
        // TODO: return iterator to node by key
        return Iterator(nullptr);
    }

protected:
    // TODO: allocate a TNode instead of Base::Node
    AVLNode* createNode(const K& key, const V& value) override {
        // TODO
        return new TNode(key, value);
    }

    // TODO: maintain threading on insertion
    void onInserted(AVLNode* /*newNode*/,
                    AVLNode* /*pred*/,
                    AVLNode* /*succ*/) override {
        // TODO
    }

    // TODO: maintain threading on deletion (0/1 child case)
    void onErasing(AVLNode* /*delNode*/,
                   AVLNode* /*pred*/,
                   AVLNode* /*succ*/) override {
        // TODO
    }

    // TODO: maintain threading when deleting a node with 2 children
    void onReplaceBySuccessor(AVLNode* /*target*/,
                              AVLNode* /*successor*/,
                              AVLNode* /*successorNext*/) override {
        // TODO
    }
};

#endif /* THREADED_AVL_H */
