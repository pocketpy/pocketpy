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

    Dict(VM* vm): vm(vm), _capacity(__Capacity),
            _mask(__Capacity-1),
            _size(0), _critical_size(__Capacity*__LoadFactor+0.5f), _head_idx(-1), _tail_idx(-1){
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64.alloc(_capacity * sizeof(ItemNode));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));
    }

    int size() const { return _size; }

    Dict(Dict&& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _head_idx = other._head_idx;
        _tail_idx = other._tail_idx;
        _items = other._items;
        _nodes = other._nodes;
        other._items = nullptr;
        other._nodes = nullptr;
    }

    Dict(const Dict& other){
        vm = other.vm;
        _capacity = other._capacity;
        _mask = other._mask;
        _size = other._size;
        _critical_size = other._critical_size;
        _head_idx = other._head_idx;
        _tail_idx = other._tail_idx;
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memcpy(_items, other._items, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64.alloc(_capacity * sizeof(ItemNode));
        memcpy(_nodes, other._nodes, _capacity * sizeof(ItemNode));
    }

    Dict& operator=(const Dict&) = delete;
    Dict& operator=(Dict&&) = delete;

    void _probe(PyObject* key, bool& ok, int& i) const;

    void set(PyObject* key, PyObject* val){
        // do possible rehash
        if(_size+1 > _critical_size) _rehash();
        bool ok; int i;
        _probe(key, ok, i);
        if(!ok) {
            _size++;
            _items[i].first = key;

            // append to tail
            if(_size == 0+1){
                _head_idx = i;
                _tail_idx = i;
            }else{
                _nodes[i].prev = _tail_idx;
                _nodes[_tail_idx].next = i;
                _tail_idx = i;
            }
        }
        _items[i].second = val;
    }

    void _rehash(){
        Item* old_items = _items;
        int old_capacity = _capacity;
        _capacity *= 2;
        _mask = _capacity - 1;
        _size = 0;
        _critical_size = _capacity*__LoadFactor+0.5f;
        _head_idx = -1;
        _tail_idx = -1;
        pool64.dealloc(_nodes);
        _items = (Item*)pool128.alloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
        _nodes = (ItemNode*)pool64.alloc(_capacity * sizeof(ItemNode));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));

        for(int i=0; i<old_capacity; i++){
            if(old_items[i].first == nullptr) continue;
            set(old_items[i].first, old_items[i].second);
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

        if(_size == 0){
            _head_idx = -1;
            _tail_idx = -1;
        }else{
            if(_head_idx == i){
                _head_idx = _nodes[i].next;
                _nodes[_head_idx].prev = -1;
            }else if(_tail_idx == i){
                _tail_idx = _nodes[i].prev;
                _nodes[_tail_idx].next = -1;
            }else{
                _nodes[_nodes[i].prev].next = _nodes[i].next;
                _nodes[_nodes[i].next].prev = _nodes[i].prev;
            }
        }
        _nodes[i].prev = -1;
        _nodes[i].next = -1;
    }

    void update(const Dict& other){
        other.apply([&](PyObject* k, PyObject* v){ set(k, v); });
    }

    template<typename __Func>
    void apply(__Func f) const {
        int i = _head_idx;
        while(i != -1){
            f(_items[i].first, _items[i].second);
            i = _nodes[i].next;
        }
    }

    Tuple keys() const{
        Tuple t(_size);
        int i = _head_idx;
        int j = 0;
        while(i != -1){
            t[j++] = _items[i].first;
            i = _nodes[i].next;
        }
        PK_ASSERT(j == _size);
        return t;
    }

    Tuple values() const{
        Tuple t(_size);
        int i = _head_idx;
        int j = 0;
        while(i != -1){
            t[j++] = _items[i].second;
            i = _nodes[i].next;
        }
        PK_ASSERT(j == _size);
        return t;
    }

    void clear(){
        _size = 0;
        _head_idx = -1;
        _tail_idx = -1;
        memset(_items, 0, _capacity * sizeof(Item));
        memset(_nodes, -1, _capacity * sizeof(ItemNode));
    }

    ~Dict(){
        if(_items==nullptr) return;
        pool128.dealloc(_items);
        pool64.dealloc(_nodes);
    }

    void _gc_mark() const{
        apply([](PyObject* k, PyObject* v){
            PK_OBJ_MARK(k);
            PK_OBJ_MARK(v);
        });
    }
};

} // namespace pkpy