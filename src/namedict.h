#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

const std::vector<uint16_t> kHashSeeds = {9629, 43049, 13267, 59509, 39251, 1249, 35803, 54469, 27689, 9719, 34897, 18973, 30661, 19913, 27919, 32143, 3467, 28019, 1051, 39419, 1361, 28547, 48197, 2609, 24317, 22861, 41467, 17623, 52837, 59053, 33589, 32117};

inline static uint16_t find_next_capacity(uint16_t n){
    uint16_t x = 2;
    while(x < n) x <<= 1;
    return x;
}

#define _hash(key, mask, hash_seed) ( ( (key).index * (hash_seed) >> 8 ) & (mask) )

inline static uint16_t find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys){
    if(keys.empty()) return kHashSeeds[0];
    std::set<uint16_t> indices;
    std::pair<uint16_t, float> best_score = {kHashSeeds[0], 0.0f};
    for(int i=0; i<kHashSeeds.size(); i++){
        indices.clear();
        for(auto key: keys){
            uint16_t index = _hash(key, capacity-1, kHashSeeds[i]);
            indices.insert(index);
        }
        float score = indices.size() / (float)keys.size();
        if(score > best_score.second) best_score = {kHashSeeds[i], score};
    }
    return best_score.first;
}

struct NameDict {
    using Item = std::pair<StrName, PyObject*>;
    static constexpr uint16_t __Capacity = 128/sizeof(Item);
    static_assert( (__Capacity & (__Capacity-1)) == 0, "__Capacity must be power of 2" );
    float _load_factor;
    uint16_t _capacity;
    uint16_t _size;
    uint16_t _hash_seed;
    uint16_t _mask;
    Item* _items;

    void _alloc(int cap){
        _items = (Item*)pool128.alloc(cap * sizeof(Item));
        memset(_items, 0, cap * sizeof(Item));
    }

    NameDict(float load_factor=0.67, uint16_t capacity=__Capacity, uint16_t hash_seed=kHashSeeds[0]):
        _load_factor(load_factor), _capacity(capacity), _size(0), 
        _hash_seed(hash_seed), _mask(capacity-1) {
        _alloc(capacity);
    }

    NameDict(const NameDict& other) {
        memcpy(this, &other, sizeof(NameDict));
        _alloc(_capacity);
        for(int i=0; i<_capacity; i++){
            _items[i] = other._items[i];
        }
    }

    NameDict& operator=(const NameDict& other) {
        pool128.dealloc(_items);
        memcpy(this, &other, sizeof(NameDict));
        _alloc(_capacity);
        for(int i=0; i<_capacity; i++){
            _items[i] = other._items[i];
        }
        return *this;
    }
    
    ~NameDict(){ pool128.dealloc(_items); }

    NameDict(NameDict&&) = delete;
    NameDict& operator=(NameDict&&) = delete;
    uint16_t size() const { return _size; }

#define HASH_PROBE(key, ok, i)          \
ok = false;                             \
i = _hash(key, _mask, _hash_seed);      \
while(!_items[i].first.empty()) {       \
    if(_items[i].first == (key)) { ok = true; break; }  \
    i = (i + 1) & _mask;                                \
}

    PyObject* operator[](StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range(fmt("NameDict key not found: ", key));
        return _items[i].second;
    }

    void set(StrName key, PyObject* val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _capacity*_load_factor){
                _rehash(true);
                HASH_PROBE(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash(bool resize){
        Item* old_items = _items;
        uint16_t old_capacity = _capacity;
        if(resize){
            _capacity = find_next_capacity(_capacity * 2);
            _mask = _capacity - 1;
        }
        _alloc(_capacity);
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_items[i].first.empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE(old_items[i].first, ok, j);
            if(ok) FATAL_ERROR();
            _items[j] = old_items[i];
        }
        pool128.dealloc(old_items);
    }

    void _try_perfect_rehash(){
        _hash_seed = find_perfect_hash_seed(_capacity, keys());
        _rehash(false); // do not resize
    }

    PyObject* try_get(StrName key) const{
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return nullptr;
        return _items[i].second;
    }

    bool try_set(StrName key, PyObject* val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return false;
        _items[i].second = val;
        return true;
    }

    bool contains(StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        return ok;
    }

    void update(const NameDict& other){
        for(uint16_t i=0; i<other._capacity; i++){
            auto& item = other._items[i];
            if(!item.first.empty()) set(item.first, item.second);
        }
    }

    void erase(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range(fmt("NameDict key not found: ", key));
        _items[i].first = StrName();
        _items[i].second = nullptr;
        _size--;
    }

    std::vector<Item> items() const {
        std::vector<Item> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            v.push_back(_items[i]);
        }
        return v;
    }

    std::vector<StrName> keys() const {
        std::vector<StrName> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            v.push_back(_items[i].first);
        }
        return v;
    }

    void _gc_mark() const;
#undef HASH_PROBE
#undef _hash
};

} // namespace pkpy