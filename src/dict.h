#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"

struct Dict{
    using Item = std::pair<PyObject*, PyObject*>;
    static constexpr int __Capacity = 8;
    static constexpr float __LoadFactor = 0.67;
    static_assert(sizeof(Item) * __Capacity <= 128);

    int _capacity;
    int _mask;
    int _size;
    int _critical_size;
    Item* _items;

    void _alloc(int cap){
        _items = (Item*)pool128.alloc(cap * sizeof(Item));
        memset(_items, 0, cap * sizeof(Item));
    }

    Dict(): _capacity(__Capacity),
            _mask(__Capacity-1),
            _size(0), _critical_size(__Capacity*__LoadFactor+0.5f){
        _alloc(__Capacity);
    }

    int size() const { return _size; }

    void _probe(PyObject* key, bool& ok, int& i){
        ok = false;
        i = PyHash(key) & _mask;
        while(_items[i].first != nullptr) {
            if(PyEquals(_items[i].first, key)) { ok = true; break; }
            i = (i + 1) & _mask;
        }
    }

    void set(PyObject* key, PyObject* val){
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _critical_size){
                _rehash();
                _probe(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash(){
        Item* old_items = _items;
        int old_capacity = _capacity;
        _capacity = find_next_power_of_2(_capacity * 2);
        _mask = _capacity - 1;
        _critical_size = _capacity * __LoadFactor + 0.5f;
        _alloc(_capacity);
        for(int i=0; i<old_capacity; i++){
            if(old_items[i].first == nullptr) continue;
            bool ok; int j;
            _probe(old_items[i].first, ok, j);
            if(ok) FATAL_ERROR();
            _items[j] = old_items[i];
        }
        pool128.dealloc(old_items);
    }

    PyObject* try_get(PyObject* key) const{
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) return nullptr;
        return _items[i].second;
    }

    ~Dict(){ pool128.dealloc(_items); }
};