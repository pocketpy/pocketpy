#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

struct Dict{
    struct Item{
        PyObject* first;
        PyObject* second;
    };

    struct ItemNode{
        int prev;
        int next;
    };

    static constexpr int __Capacity = 8;
    static constexpr float __LoadFactor = 0.67f;
    static_assert(sizeof(Item) * __Capacity <= 128);
    static_assert(sizeof(ItemNode) * __Capacity <= 64);

    VM* vm;
    int _capacity;
    int _mask;
    int _size;
    int _critical_size;
    int _head_idx;          // for order preserving
    int _tail_idx;          // for order preserving
    Item* _items;
    ItemNode* _nodes;       // for order preserving

    Dict(VM* vm);
    Dict(Dict&& other);
    Dict(const Dict& other);
    Dict& operator=(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;

    int size() const { return _size; }

    void _probe_0(PyObject* key, bool& ok, int& i) const;
    void _probe_1(PyObject* key, bool& ok, int& i) const;

    void set(PyObject* key, PyObject* val);
    void _rehash();

    PyObject* try_get(PyObject* key) const;

    bool contains(PyObject* key) const;
    bool erase(PyObject* key);
    void update(const Dict& other);

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