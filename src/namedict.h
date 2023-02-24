#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{
    const std::vector<uint32_t> kHashSeeds = {2654435761, 740041872, 89791836, 2530921597, 3099937610, 4149637300, 2701344377, 1871341841, 1162794509, 172427115, 1636841237, 716883023, 3294650677, 54921151, 3697702254, 632800580, 704251301, 1107400416, 3158440428, 581874317, 3196521560, 2374935651, 3196227762, 2033551959, 2028119291, 348132418, 392150876, 3839168722, 3705071505, 742931757, 2917622539, 3641634736, 3438863246, 1211314974, 1389620692, 3842835768, 165823282, 2225611914, 1862128271, 2147948325, 3759309280, 2087364973, 3453466014, 2082604761, 3627961499, 967790220, 3285133283, 2749567844, 262853493, 142639230, 3079101350, 2942333634, 1470374050, 3719337124, 2487858314, 1605159164, 2958061235, 3310454023, 3143584575, 3696188862, 3455413544, 148400163, 889426286, 1485235735};

    uint32_t find_next_capacity(uint32_t n){
        uint32_t x = 2;
        while(x < n) x *= 2;
        return x;
    }

#define _hash(key, mask, hash_seed) ( ( (key).index * (hash_seed) >> 16 ) & (mask) )

    uint32_t find_perfect_hash_seed(uint32_t capacity, const std::vector<StrName>& keys){
        if(keys.empty()) return kHashSeeds[0];
        std::set<uint32_t> indices;
        std::vector<std::pair<uint32_t, float>> scores;
        for(int i=0; i<kHashSeeds.size(); i++){
            indices.clear();
            for(auto key: keys){
                uint32_t index = _hash(key, capacity-1, kHashSeeds[i]);
                indices.insert(index);
            }
            float score = indices.size() / (float)keys.size();
            scores.push_back({kHashSeeds[i], score});
        }
        std::sort(scores.begin(), scores.end(), [](auto a, auto b){ return a.second > b.second; });
        return scores[0].first;
    }

    struct NameDictNode{
        StrName first;
        PyVar second;
        inline bool empty() const { return first.empty(); }
    };

    struct NameDict {
        uint32_t _capacity;
        uint32_t _size;
        float _load_factor;
        uint32_t _hash_seed;
        NameDictNode* _a;
        uint32_t _mask;

        NameDict(uint32_t capacity=2, float load_factor=0.67, uint32_t hash_seed=kHashSeeds[0]):
            _capacity(capacity), _size(0), _load_factor(load_factor),
            _hash_seed(hash_seed), _a(new NameDictNode[capacity]), _mask(capacity-1) {}

        NameDict(const NameDict& other) {
            this->_capacity = other._capacity;
            this->_size = other._size;
            this->_load_factor = other._load_factor;
            this->_hash_seed = other._hash_seed;
            this->_a = new NameDictNode[_capacity];
            this->_mask = other._mask;
            for(uint32_t i=0; i<_capacity; i++) _a[i] = other._a[i];
        }
        
        NameDict& operator=(const NameDict&) = delete;
        NameDict(NameDict&&) = delete;
        NameDict& operator=(NameDict&&) = delete;

        uint32_t size() const { return _size; }

#define HASH_PROBE(key, ok, i) \
    ok = false; \
    i = _hash(key, _mask, _hash_seed); \
    while(!_a[i].empty()) { \
        if(_a[i].first == (key)) { ok = true; break; } \
        i = (i + 1) % _capacity; \
    }

        const PyVar& operator[](StrName key) const {
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
            return _a[i].second;
        }

        PyVar& get(StrName key){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
            return _a[i].second;
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
                _a[i].first = key;
            }
            _a[i].second = std::forward<T>(value);
        }

        void _rehash(bool resize){
            NameDictNode* old_a = _a;
            uint32_t old_capacity = _capacity;
            if(resize){
                _capacity = find_next_capacity(_capacity * 2);
                _mask = _capacity - 1;
            }
            _a = new NameDictNode[_capacity];
            for(uint32_t i=0; i<old_capacity; i++){
                if(old_a[i].empty()) continue;
                bool ok; uint32_t j;
                HASH_PROBE(old_a[i].first, ok, j);
                if(ok) UNREACHABLE();
                _a[j] = std::move(old_a[i]);
            }
            delete[] old_a;
        }

        void _try_perfect_rehash(){
            _hash_seed = find_perfect_hash_seed(_capacity, keys());
            _rehash(false); // do not resize
        }

        inline PyVar* try_get(StrName key){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) return nullptr;
            return &_a[i].second;
        }

        inline bool try_set(StrName key, PyVar&& value){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) return false;
            _a[i].second = std::move(value);
            return true;
        }

        inline bool contains(StrName key) const {
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            return ok;
        }

        ~NameDict(){ delete[] _a; }

        void update(const NameDict& other){
            for(uint32_t i=0; i<other._capacity; i++){
                if(other._a[i].empty()) continue;
                set(other._a[i].first, other._a[i].second);
            }
        }

        void erase(StrName key){
            bool ok; uint32_t i;
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found: " + key.str());
            _a[i] = NameDictNode();
            _size--;
        }

        std::vector<NameDictNode> items() const {
            std::vector<NameDictNode> v;
            for(uint32_t i=0; i<_capacity; i++){
                if(_a[i].empty()) continue;
                v.push_back(_a[i]);
            }
            return v;
        }

        std::vector<StrName> keys() const {
            std::vector<StrName> v;
            for(uint32_t i=0; i<_capacity; i++){
                if(_a[i].empty()) continue;
                v.push_back(_a[i].first);
            }
            return v;
        }
#undef HASH_PROBE
#undef _hash
    };

} // namespace pkpy