#pragma once

#include "obj.h"
#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

struct Dict{
    using Item = std::pair<PyObject*, PyObject*>;
    static constexpr int __Capacity = 8;
    static constexpr float __LoadFactor = 0.67f;
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

    Dict(Dict&& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _items = other._items;
        other._items = nullptr;
    }

    Dict(const Dict& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memcpy(_items, other._items, _capacity * sizeof(Item));
    }

    Dict& operator=(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;

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

    bool contains(PyObject* key) const{
        bool ok; int i;
        _probe(key, ok, i);
        return ok;
    }

    void erase(PyObject* key){
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) return;
        _items[i].first = nullptr;
        _items[i].second = nullptr;
        _size--;
    }

    void update(const Dict& other){
        for(int i=0; i<other._capacity; i++){
            if(other._items[i].first == nullptr) continue;
            set(other._items[i].first, other._items[i].second);
        }
    }

    std::vector<Item> items() const {
        std::vector<Item> v;
        for(int i=0; i<_capacity; i++){
            if(_items[i].first == nullptr) continue;
            v.push_back(_items[i]);
        }
        return v;
    }

    template<typename __Func>
    void apply(__Func f) const {
        for(int i=0; i<_capacity; i++){
            if(_items[i].first == nullptr) continue;
            f(_items[i].first, _items[i].second);
        }
    }

    void clear(){
        memset(_items, 0, _capacity * sizeof(Item));
        _size = 0;
    }

    ~Dict(){ if(_items!=nullptr) pool128.dealloc(_items); }

    void _gc_mark() const{
        for(int i=0; i<_capacity; i++){
            if(_items[i].first == nullptr) continue;
            PK_OBJ_MARK(_items[i].first);
            PK_OBJ_MARK(_items[i].second);
        }
    }
};

} // namespace pkpy