#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

struct Dict{
    struct Item{
        PyVar first;
        PyVar second;
    };

    struct ItemNode{
        int prev;
        int next;
    };

    static constexpr int __Capacity = 4;
    static constexpr float __LoadFactor = 0.67f;
    static_assert(sizeof(Item) * __Capacity <= 128);
    static_assert(sizeof(ItemNode) * __Capacity <= 64);

    int _capacity;
    int _mask;
    int _size;
    int _critical_size;
    int _head_idx;          // for order preserving
    int _tail_idx;          // for order preserving
    Item* _items;
    ItemNode* _nodes;       // for order preserving

    Dict();
    Dict(Dict&& other);
    Dict(const Dict& other);
    Dict& operator=(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;

    int size() const { return _size; }

    void _probe_0(VM* vm, PyVar key, bool& ok, int& i) const;
    void _probe_1(VM* vm, PyVar key, bool& ok, int& i) const;

    void set(VM* vm, PyVar key, PyVar val);
    void _rehash(VM* vm);

    PyVar try_get(VM* vm, PyVar key) const;

    bool contains(VM* vm, PyVar key) const;
    bool erase(VM* vm, PyVar key);
    void update(VM* vm, const Dict& other);

    template<typename __Func>
    void apply(__Func f) const {
        int i = _head_idx;
        while(i != -1){
            f(_items[i].first, _items[i].second);
            i = _nodes[i].next;
        }
    }

    Tuple keys() const;
    Tuple values() const;
    void clear();
    ~Dict();

    void _gc_mark() const;
};

} // namespace pkpy