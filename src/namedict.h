#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{
    const std::vector<uint32_t> kHashSeeds = {0, 3259656564, 3106121857, 2774518055, 4085946151, 4274771677, 4047908201, 2149081045, 4160441109, 4127125901, 3109730425, 2794687362, 2806137727, 2642447290, 4070996945, 3580743775, 3719956858, 2960278187, 3568486238, 3125361093, 2232173865, 4043238260, 3265527710, 2206062780, 3968387223, 3144295694, 3293736932, 3196583945, 3832534010, 3311528523, 4258510773, 4049882022, 3058077580, 2446794117, 2330081744, 2563269634, 3848248775, 2197398712, 2874906918, 3012473024, 3477039876, 2710692860, 2806508231, 3893239503, 3929140074, 3145323261, 3593960112, 2451662716, 2545939029, 2475647797, 2790321726, 4166873680, 3504262692, 3140715282, 3078827310, 3177714229, 3006241931, 3777800785, 3621627818, 3163832382, 2166076714, 3622591406, 3299007679, 2915427082, 3939911590, 4145015468, 2791077264, 3916399405, 3330576709, 2466029172, 3534773842, 2690327419, 2487859383, 3687001303, 2615131117, 3057598651, 2548471802, 3145782646, 3895406770, 2150621965, 2179753887, 2159855306, 2439700132, 2397760304, 3405860607, 4268549710, 2779408554, 2485874456, 3796299954, 4179315997, 2380599704, 3210079474, 3951990603, 3342489194, 2997361581, 3576131817, 3163713423, 2467495451, 4190562029, 2588496185};
    const std::vector<uint32_t> kPrimes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599};

    uint32_t find_next_prime(uint32_t n){
        auto it = std::lower_bound(kPrimes.begin(), kPrimes.end(), n);
        if(it == kPrimes.end()) return n;
        return *it;
    }

    inline uint32_t _hash(StrName key, uint32_t capacity, uint32_t hash_seed){
        uint32_t i = key.index * (uint32_t)2654435761;
        return (i ^ hash_seed) % capacity;
    }

    uint32_t find_perfect_hash_seed(uint32_t capacity, const std::vector<StrName>& keys){
        if(keys.empty()) return 0;
        std::set<uint32_t> indices;
        std::vector<std::pair<uint32_t, float>> scores;
        for(int i=0; i<kHashSeeds.size(); i++){
            indices.clear();
            for(auto key: keys){
                uint32_t index = _hash(key, capacity, kHashSeeds[i]);
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

        NameDict(uint32_t capacity=2, float load_factor=0.67, uint32_t hash_seed=0):
            _capacity(capacity), _size(0), _load_factor(load_factor),
            _hash_seed(hash_seed), _a(new NameDictNode[capacity]) {}

        NameDict(const NameDict& other) {
            this->_capacity = other._capacity;
            this->_size = other._size;
            this->_load_factor = other._load_factor;
            this->_hash_seed = other._hash_seed;
            this->_a = new NameDictNode[_capacity];
            for(uint32_t i=0; i<_capacity; i++) _a[i] = other._a[i];
        }
        
        NameDict& operator=(const NameDict&) = delete;
        NameDict(NameDict&&) = delete;
        NameDict& operator=(NameDict&&) = delete;

        uint32_t size() const { return _size; }

#define HASH_PROBE(key, ok, i) \
    bool ok = false; uint32_t i; \
    i = _hash(key, _capacity, _hash_seed); \
    while(!_a[i].empty()) { \
        if(_a[i].first == (key)) { ok = true; break; } \
        i = (i + 1) % _capacity; \
    }

#define HASH_PROBE_OVERRIDE(key, ok, i) \
    ok = false; \
    i = _hash(key, _capacity, _hash_seed); \
    while(!_a[i].empty()) { \
        if(_a[i].first == (key)) { ok = true; break; } \
        i = (i + 1) % _capacity; \
    }

        const PyVar& operator[](StrName key) const {
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found");
            return _a[i].second;
        }

        [[nodiscard]] PyVar& operator[](StrName key){
            HASH_PROBE(key, ok, i);
            if(!ok) {
                _a[i].first = key;
                _size++;
                if(_size > _capacity * _load_factor){
                    _rehash(true);
                    HASH_PROBE_OVERRIDE(key, ok, i);
                }
            }
            return _a[i].second;
        }

        void _rehash(bool resize){
            NameDictNode* old_a = _a;
            uint32_t old_capacity = _capacity;
            if(resize) _capacity = find_next_prime(_capacity * 2);
            _size = 0;
            _a = new NameDictNode[_capacity];
            for(uint32_t i=0; i<old_capacity; i++){
                if(old_a[i].empty()) continue;
                HASH_PROBE(old_a[i].first, ok, j);
                if(ok) UNREACHABLE();
                _a[j].first = old_a[i].first;
                _a[j].second = std::move(old_a[i].second);
                _size++;
            }
            delete[] old_a;
        }

        void _try_perfect_rehash(){
            std::vector<StrName> keys;
            for(uint32_t i=0; i<_capacity; i++){
                if(_a[i].empty()) continue;
                keys.push_back(_a[i].first);
            }
            _hash_seed = find_perfect_hash_seed(_capacity, keys);
            _rehash(false); // do not resize
        }

        inline PyVar* try_get(StrName key){
            HASH_PROBE(key, ok, i);
            if(!ok) return nullptr;
            return &_a[i].second;
        }

        inline bool try_set(StrName key, PyVar&& value){
            HASH_PROBE(key, ok, i);
            if(!ok) return false;
            _a[i].second = std::move(value);
            return true;
        }

        inline bool contains(StrName key) const {
            HASH_PROBE(key, ok, i);
            return ok;
        }

        ~NameDict(){ delete[] _a;}

        struct iterator {
            const NameDict* _dict;
            uint32_t i;
            iterator() = default;
            iterator(const NameDict* dict, uint32_t i): _dict(dict), i(i) { _skip_empty(); }
            inline void _skip_empty(){ while(i < _dict->_capacity && _dict->_a[i].empty()) i++;}
            inline iterator& operator++(){ i++; _skip_empty(); return *this;}

            inline bool operator!=(const iterator& other) const { return i != other.i; }
            inline bool operator==(const iterator& other) const { return i == other.i; }

            inline NameDictNode* operator->() const { return &_dict->_a[i]; }
        };

        template<typename T>
        void emplace(StrName key, T&& value){
            HASH_PROBE(key, ok, i);
            if(!ok) {
                _a[i].first = key;
                _size++;
                if(_size > _capacity * _load_factor){
                    _rehash(true);
                    HASH_PROBE_OVERRIDE(key, ok, i);
                }
            }
            _a[i].second = std::forward<T>(value);
        }

        void insert(iterator begin, iterator end){
            for(auto it = begin; it != end; ++it){
                emplace(it->first, it->second);
            }
        }

        iterator find(StrName key) const{
            HASH_PROBE(key, ok, i);
            if(!ok) return end();
            return iterator(this, i);
        }

        void erase(StrName key){
            HASH_PROBE(key, ok, i);
            if(!ok) throw std::out_of_range("NameDict key not found");
            _a[i] = NameDictNode();
            _size--;
        }

        inline iterator begin() const { return iterator(this, 0); }
        inline iterator end() const { return iterator(this, _capacity); }

    #undef HASH_PROBE
    #undef HASH_PROBE_OVERRIDE
    };

} // namespace pkpy