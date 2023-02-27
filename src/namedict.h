#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

const int kNameDictNodeSize = sizeof(StrName) + sizeof(PyVar);

template<int __Bucket, int __BucketSize=32>
struct DictArrayPool {
    std::vector<StrName*> buckets[__Bucket+1];

    StrName* alloc(uint16_t n){
        StrName* _keys;
        if(n > __Bucket || buckets[n].empty()){
            _keys = (StrName*)malloc(kNameDictNodeSize * n);
            memset((void*)_keys, 0, kNameDictNodeSize * n);
        }else{
            _keys = buckets[n].back();
            memset((void*)_keys, 0, sizeof(StrName) * n);
            buckets[n].pop_back();
        }
        return _keys;
    }

    void dealloc(StrName* head, uint16_t n){
        PyVar* _values = (PyVar*)(head + n);
        if(n > __Bucket || buckets[n].size() >= __BucketSize){
            for(int i=0; i<n; i++) _values[i].~PyVar();
            free(head);
        }else{
            buckets[n].push_back(head);
        }
    }

    ~DictArrayPool(){
        // let it leak, since this object is static
    }
};

const std::vector<uint16_t> kHashSeeds = {9629, 43049, 13267, 59509, 39251, 1249, 35803, 54469, 27689, 9719, 34897, 18973, 30661, 19913, 27919, 32143, 3467, 28019, 1051, 39419, 1361, 28547, 48197, 2609, 24317, 22861, 41467, 17623, 52837, 59053, 33589, 32117};
static DictArrayPool<32> _dict_pool;

uint16_t find_next_capacity(uint16_t n){
    uint16_t x = 2;
    while(x < n) x <<= 1;
    return x;
}

#define _hash(key, mask, hash_seed) ( ( (key).index * (hash_seed) >> 8 ) & (mask) )

uint16_t find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys){
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
    uint16_t _capacity;
    uint16_t _size;
    float _load_factor;
    uint16_t _hash_seed;
    uint16_t _mask;
    StrName* _keys;

    inline PyVar& value(uint16_t i){
        return reinterpret_cast<PyVar*>(_keys + _capacity)[i];
    }

    inline const PyVar& value(uint16_t i) const {
        return reinterpret_cast<const PyVar*>(_keys + _capacity)[i];
    }

    NameDict(uint16_t capacity=2, float load_factor=0.67, uint16_t hash_seed=kHashSeeds[0]):
        _capacity(capacity), _size(0), _load_factor(load_factor),
        _hash_seed(hash_seed), _mask(capacity-1) {
            _keys = _dict_pool.alloc(capacity);
        }

    NameDict(const NameDict& other) {
        memcpy(this, &other, sizeof(NameDict));
        _keys = _dict_pool.alloc(_capacity);
        for(int i=0; i<_capacity; i++){
            _keys[i] = other._keys[i];
            value(i) = other.value(i);
        }
    }

    NameDict& operator=(const NameDict& other) {
        _dict_pool.dealloc(_keys, _capacity);
        memcpy(this, &other, sizeof(NameDict));
        _keys = _dict_pool.alloc(_capacity);
        for(int i=0; i<_capacity; i++){
            _keys[i] = other._keys[i];
            value(i) = other.value(i);
        }
        return *this;
    }
    
    ~NameDict(){ _dict_pool.dealloc(_keys, _capacity); }

    NameDict(NameDict&&) = delete;
    NameDict& operator=(NameDict&&) = delete;
    uint16_t size() const { return _size; }

#define HASH_PROBE(key, ok, i) \
ok = false; \
i = _hash(key, _mask, _hash_seed); \
while(!_keys[i].empty()) { \
    if(_keys[i] == (key)) { ok = true; break; } \
    i = (i + 1) & _mask; \
}

    const PyVar& operator[](StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
        return value(i);
    }

    PyVar& get(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
        return value(i);
    }

    template<typename T>
    void set(StrName key, T&& val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _capacity*_load_factor){
                _rehash(true);
                HASH_PROBE(key, ok, i);
            }
            _keys[i] = key;
        }
        value(i) = std::forward<T>(val);
    }

    void _rehash(bool resize){
        StrName* old_keys = _keys;
        PyVar* old_values = &value(0);
        uint16_t old_capacity = _capacity;
        if(resize){
            _capacity = find_next_capacity(_capacity * 2);
            _mask = _capacity - 1;
        }
        _keys = _dict_pool.alloc(_capacity);
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_keys[i].empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE(old_keys[i], ok, j);
            if(ok) UNREACHABLE();
            _keys[j] = old_keys[i];
            value(j) = old_values[i]; // std::move makes a segfault
        }
        _dict_pool.dealloc(old_keys, old_capacity);
    }

    void _try_perfect_rehash(){
        _hash_seed = find_perfect_hash_seed(_capacity, keys());
        _rehash(false); // do not resize
    }

    inline PyVar* try_get(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return nullptr;
        return &value(i);
    }

    inline bool try_set(StrName key, PyVar&& val){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) return false;
        value(i) = std::move(val);
        return true;
    }

    inline bool contains(StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        return ok;
    }

    void update(const NameDict& other){
        for(uint16_t i=0; i<other._capacity; i++){
            if(other._keys[i].empty()) continue;
            set(other._keys[i], other.value(i));
        }
    }

    void erase(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE(key, ok, i);
        if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
        _keys[i] = StrName(); value(i).reset();
        _size--;
    }

    std::vector<std::pair<StrName, PyVar>> items() const {
        std::vector<std::pair<StrName, PyVar>> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_keys[i].empty()) continue;
            v.push_back(std::make_pair(_keys[i], value(i)));
        }
        return v;
    }

    std::vector<StrName> keys() const {
        std::vector<StrName> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_keys[i].empty()) continue;
            v.push_back(_keys[i]);
        }
        return v;
    }
#undef HASH_PROBE
#undef _hash
};

} // namespace pkpy