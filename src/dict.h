#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

struct Dict{
    using Item = std::pair<PyObject*, PyObject*>;
    static constexpr int __Capacity = 8;
    static constexpr float __LoadFactor = 0.67;
    static_assert(sizeof(Item) * __Capacity <= 128);

    VM* vm;
    int _capacity;
    int _mask;
    int _size;
    int _critical_size;
    Item* _items;
    
    Dict(VM* vm): vm(vm), _capacity(__Capacity),
            _mask(__Capacity-1),
            _size(0), _critical_size(__Capacity*__LoadFactor+0.5f){
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
    }

    int size() const { return _size; }

    void _probe(PyObject* key, bool& ok, int& i) const;

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
        _capacity *= 2;
        _mask = _capacity - 1;
        _critical_size = _capacity * __LoadFactor + 0.5f;
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
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

} // namespace pkpy