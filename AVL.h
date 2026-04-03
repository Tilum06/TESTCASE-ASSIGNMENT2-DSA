#ifndef AVL_H
#define AVL_H

#include "IBST.h"

#include <list>
#include <string>

template <typename K, typename V>
class AVL : public IBST<K, V> {
public:
    struct Node {
        K key;
        V value;

        int bfactor; 

        Node* pLeft;
        Node* pRight;

        Node(const K& k, const V& v)
            : key(k), value(v), bfactor(0),
              pLeft(nullptr), pRight(nullptr) {}

        virtual ~Node() = default;

        // Default formatter must compile for any K,V.
        static std::string defaultEntry2Str(const K& k, const V& v) {
            std::ostringstream oss;
            oss << "<" << k << ", " << v << ">";
            return oss.str();
        }

        std::string toString(std::string (*entry2str)(const K&, const V&) = nullptr) const {
            auto entryStr = [&](const K& kk, const V& vv) -> std::string {
                return entry2str ? entry2str(kk, vv) : defaultEntry2Str(kk, vv);
            };

            std::ostringstream out;
            // AVL luôn in kèm balance cho debug
            if (!pLeft && !pRight) {
                out << "[" << entryStr(key, value) << ":" << balance() << "]";
            }
            else if (!pLeft && pRight) {
                out << " (" << entryStr(key, value) << ":" << balance()
                    << "[.]" << pRight->toString(entry2str) << ")";
            }
            else if (pLeft && !pRight) {
                out << " (" << entryStr(key, value) << ":" << balance()
                    << pLeft->toString(entry2str) << "[.]" << ")";
            }
            else {
                out << " (" << entryStr(key, value) << ":" << balance()
                    << pLeft->toString(entry2str)
                    << pRight->toString(entry2str)
                    << ") ";
            }
            return out.str();
        }
    };

protected:
    Node* pRoot;

protected:
    // =======================
    // Hooks for ThreadedAVL
    // =======================
    // Base AVL does nothing. ThreadedAVL overrides these to maintain prev/next threading.

    virtual void onInserted(Node* /*newNode*/, Node* /*pred*/, Node* /*succ*/) {}
    virtual void onErasing(Node* /*delNode*/, Node* /*pred*/, Node* /*succ*/) {}
    virtual void onReplaceBySuccessor(Node* /*target*/, Node* /*successor*/, Node* /*successorNext*/) {}

    // Factory for subclasses to create extended node types
    virtual Node* createNode(const K& key, const V& value) { return new Node(key, value); }

  
public:
    AVL() : pRoot(nullptr) {}
    virtual ~AVL() { clear(); }

    // =======================
    // IBST implementation (TODO)
    // =======================

    bool insert(const K& key, const V& value) override {
        // TODO
        (void)key; (void)value;
        return false;
    }

    bool erase(const K& key) override {
        // TODO
        (void)key;
        return false;
    }

    V* find(const K& key) override {
        // TODO
        (void)key;
        return nullptr;
    }

    bool contains(const K& key) const override {
        // TODO
        (void)key;
        return false;
    }

    int size() const override {
        // TODO
        return 0;
    }

    bool empty() const override {
        // TODO
        return true;
    }

    void clear() override {
        // TODO
    }

    int height() const override {
        // TODO
        return 0;
    }

    std::list<K> ascendingList() override {
        // TODO
        return {};
    }

    std::list<K> descendingList() override {
        // TODO
        return {};
    }

    std::string toString(std::string (*entry2str)(const K&, const V&) = nullptr) const override {
        if (!pRoot) return "(NULL)";
        return pRoot->toString(entry2str);
    }
};

#endif /* AVL_H */
