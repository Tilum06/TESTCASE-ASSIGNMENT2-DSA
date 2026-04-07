#ifndef IBST_H
#define IBST_H

#include <list>
#include <string>

template <typename K, typename V>
class IBST {
public:
    virtual ~IBST() = default;
    virtual bool insert(const K& key, const V& value) = 0;
    virtual bool erase(const K& key) = 0;

    virtual V* find(const K& key) = 0;

    virtual bool contains(const K& key) const = 0;

    virtual int size() const = 0;
    virtual bool empty() const = 0;
    virtual void clear() = 0;
    virtual int height() const = 0;

    virtual std::list<K> ascendingList() const = 0;
    virtual std::list<K> descendingList() const = 0;

    virtual std::string toString(
        std::string (*entry2str)(const K&, const V&) = nullptr
    ) const = 0;
};

#endif /* IBST_H */
