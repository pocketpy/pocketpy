#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"


template<int __Bucket, int __BucketSize=32>
struct DictArrayPool {
    std::deque<StrName*> buckets[__Bucket+1];

    StrName* alloc(uint32_t n){
        StrName* _keys;
        if(n > __Bucket || buckets[n].empty()){
            _keys = (StrName*)malloc((sizeof(StrName)+sizeof(PyVar)) * n);
            memset((void*)_keys, 0, (sizeof(StrName)+sizeof(PyVar)) * n);
        }else{
            _keys = buckets[n].back();
            memset((void*)_keys, 0, sizeof(StrName) * n);
            buckets[n].pop_back();
        }
        return _keys;
    }

    void dealloc(StrName* head, uint32_t n){
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

template<typename T>
class arrayset {
  public:
    arrayset(size_t capacity) {
        elements.reserve(capacity);
    }

    void insert(const T& elem) {
        for (size_t i = 0; i < elements.size(); i++) {
            if (elements[i] == elem) {
                return;
            }
        }
        elements.push_back(elem);
    }

    void clear() {
        elements.clear();
    }

    size_t size() {
        return elements.size();
    }

  private:
  std::vector<T> elements;
};

namespace pkpy{
    const std::vector<uint32_t> kHashSeeds = {2654435761, 740041872, 89791836, 2530921597, 3099937610, 4149637300, 2701344377, 1871341841, 1162794509, 172427115, 1636841237, 716883023, 3294650677, 54921151, 3697702254, 632800580, 704251301, 1107400416, 3158440428, 581874317, 3196521560, 2374935651, 3196227762, 2033551959, 2028119291, 348132418, 392150876, 3839168722, 3705071505, 742931757, 2917622539, 3641634736, 3438863246, 1211314974, 1389620692, 3842835768, 165823282, 2225611914, 1862128271, 2147948325, 3759309280, 2087364973, 3453466014, 2082604761, 3627961499, 967790220, 3285133283, 2749567844, 262853493, 142639230, 3079101350, 2942333634, 1470374050, 3719337124, 2487858314, 1605159164, 2958061235, 3310454023, 3143584575, 3696188862, 3455413544, 148400163, 889426286, 1485235735};
    static DictArrayPool<32> _dict_pool;

    uint32_t find_next_capacity(uint32_t n){
        uint32_t x = 2;
        while(x < n) x *= 2;
        return x;
    }

#define _hash(key, mask, hash_seed) ( ( (key).index * (hash_seed) >> 16 ) & (mask) )

    uint32_t find_perfect_hash_seed(uint32_t capacity, const std::vector<StrName>& keys){
        if(keys.empty()) return kHashSeeds[0];
        arrayset<uint32_t> indices(kHashSeeds.size());
        uint32_t best_seed = 0;
        float best_score = std::numeric_limits<float>::max();
        for(int i=0; i<kHashSeeds.size(); i++){
            indices.clear();
            for(auto& key: keys){
                uint32_t index = _hash(key, capacity-1, kHashSeeds[i]);
                indices.insert(index);
            }
            float score = indices.size() / (float)keys.size();
            if (score > best_score) {
                best_score = score;
                best_seed = kHashSeeds[i];
            }
        }
        return best_seed;
    }

    struct NameDict {
        uint32_t _capacity;
        uint32_t _size;
        float _load_factor;
        uint32_t _hash_seed;
        uint32_t _mask;
        StrName* _keys;
        PyVar* _values;

        inline void _alloc(uint32_t capacity){
            _keys = _dict_pool.alloc(capacity);
            _values = (PyVar*)(_keys + capacity);
        }

        inline void _dealloc(StrName* head, uint32_t capacity){
            _dict_pool.dealloc(head, capacity);
        }

        NameDict(uint32_t capacity=2, float load_factor=0.67, uint32_t hash_seed=kHashSeeds[0]):
            _capacity(capacity), _size(0), _load_factor(load_factor),
            _hash_seed(hash_seed), _mask(capacity-1) {
                _alloc(capacity);
            }

        NameDict(const NameDict& other) {
            this->_capacity = other._capacity;
            this->_size = other._size;
            this->_load_factor = other._load_factor;
            this->_hash_seed = other._hash_seed;
            _alloc(_capacity);
            this->_mask = other._mask;
            for(uint32_t i=0; i<_capacity; i++){
                _keys[i] = other._keys[i];
                _values[i] = other._values[i];
            }
        }
        
        NameDict& operator=(const NameDict&) = delete;
        NameDict(NameDict&&) = delete;
        NameDict& operator=(NameDict&&) = delete;

        uint32_t size() const { return _size; }

#define HASH_PROBE(key, ok, i) \
    ok = false; \
    i = _hash(key, _mask, _hash_seed); \
    while(!_keys[i].empty()) { \
        if(_keys[i] == (key)) { ok = true; break; } \
        i = (i + 1) & _mask; \
    }

        const PyVar& operator[](StrName key) const {
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
            return _values[i];
        }

        PyVar& get(StrName key){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
            return _values[i];
        }

        template<typename T>
        void set(StrName key, T&& value){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) {
                _size++;
                if(_size > _capacity*_load_factor){
                    _rehash(true);
                    HASH_PROBE(key, ok, i);
                }
                _keys[i] = key;
            }
            _values[i] = std::forward<T>(value);
        }

        void _rehash(bool resize){
            StrName* old_keys = _keys;
            PyVar* old_values = _values;
            uint32_t old_capacity = _capacity;
            if(resize){
                _capacity = find_next_capacity(_capacity * 2);
                _mask = _capacity - 1;
            }
            _alloc(_capacity);
            for(uint32_t i=0; i<old_capacity; i++){
                if(old_keys[i].empty()) continue;
                bool ok; uint32_t j;
                HASH_PROBE(old_keys[i], ok, j);
                if(ok) UNREACHABLE();
                _keys[j] = old_keys[i];
                _values[j] = old_values[i]; // std::move makes a segfault
            }
            _dealloc(old_keys, old_capacity);
        }

        void _try_perfect_rehash(){
            _hash_seed = find_perfect_hash_seed(_capacity, keys());
            _rehash(false); // do not resize
        }

        inline PyVar* try_get(StrName key){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) return nullptr;
            return &_values[i];
        }

        inline bool try_set(StrName key, PyVar&& value){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) return false;
            _values[i] = std::move(value);
            return true;
        }

        inline bool contains(StrName key) const {
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            return ok;
        }

        ~NameDict(){ _dealloc(_keys, _capacity); }

        void update(const NameDict& other){
            for(uint32_t i=0; i<other._capacity; i++){
                if(other._keys[i].empty()) continue;
                set(other._keys[i], other._values[i]);
            }
        }

        void erase(StrName key){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
            _keys[i] = StrName();
            _values[i].reset();
            _size--;
        }

        std::vector<std::pair<StrName, PyVar>> items() const {
            std::vector<std::pair<StrName, PyVar>> v;
            for(uint32_t i=0; i<_capacity; i++){
                if(_keys[i].empty()) continue;
                v.push_back(std::make_pair(_keys[i], _values[i]));
            }
            return v;
        }

        std::vector<StrName> keys() const {
            std::vector<StrName> v;
            for(uint32_t i=0; i<_capacity; i++){
                if(_keys[i].empty()) continue;
                v.push_back(_keys[i]);
            }
            return v;
        }
#undef HASH_PROBE
#undef _hash
    };

} // namespace pkpy