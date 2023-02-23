#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{
    const std::vector<uint32_t> kHashSeeds = {0, 2619911537, 3657312521, 3108729646, 3527288759, 3054146033, 3436461329, 3073779540, 2262929533, 3564928174, 2823402058, 4053700272, 3710518398, 2193895465, 3616465673, 2370151435, 3911946797, 2518033560, 4090755824, 2554076140, 2922670102, 2817437464, 3058134240, 4015911568, 2683197236, 3580292421, 2489282276, 2198476149, 3059557576, 3251314740, 2164089808, 3407729628, 4006319879, 3563204365, 2959032770, 3699872774, 3285955347, 2886756578, 2727979131, 3987926730, 3558848942, 3667783180, 3427603538, 2678804156, 3899695574, 3497073252, 4125590716, 3439003736, 3166960007, 2341256547, 3498025667, 2434275284, 2294495502, 2454032734, 2622845447, 2237894924, 4127773463, 2899873446, 3826047724, 2772822995, 4021041972, 3585330008, 3442671856, 4033639492, 4190375370, 3423510541, 3993284300, 3399740404, 2346010479, 2665226039, 3989420676, 2430396952, 4162553639, 3318451871, 2451157282, 3888084520, 4216786107, 3630490447, 3686500437, 4270289137, 2845436680, 3990477872, 3386727112, 2603155603, 2533548133, 2476236382, 2752268515, 2714540624, 3649552071, 2486775129, 3447438497, 2660214659, 3171847655, 2173117107, 2777204947, 3473126570, 2874563719, 3710212439, 3882999260, 3884415651, 3939886653, 2513961523, 3259070705, 4076001992, 3695924943, 2630642728, 2302962913, 3977147010, 4229898948, 3278694988, 3668138471, 4174657761, 2681204139, 2468496171, 3953941369, 4216451258, 3986080889, 3355338704, 3484226746, 3964851958, 4063196140, 3210555673, 3972895759, 2762823957};
    const std::vector<uint32_t> kPrimes = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353};

    uint32_t find_next_capacity(uint32_t n){
        auto it = std::lower_bound(kPrimes.begin(), kPrimes.end(), n);
        if(it == kPrimes.end()) return n;
        return *it;
    }

    inline uint32_t _hash(StrName key, uint32_t capacity, uint32_t hash_seed){
        return (key.index ^ hash_seed) % capacity;
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
            float find_hit_score = indices.size() / (float)keys.size();
            std::vector<uint32_t> indices_vec(indices.begin(), indices.end());
            std::sort(indices_vec.begin(), indices_vec.end());
            float find_miss_score = indices.size();
            for(int j=1; j<indices_vec.size(); j++){
                int gap = indices_vec[j] - indices_vec[j-1];
                if(gap == 1) find_miss_score -= 1;
            }
            float score = find_hit_score*2 + find_miss_score/indices.size();
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
            _hash_seed(hash_seed), _a(new NameDictNode[_capacity]) {}

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
    ok = false; \
    i = _hash(key, _capacity, _hash_seed); \
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
            if(resize) _capacity = find_next_capacity(_capacity * 2);
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

        void print_stats(){
            std::map<StrName, uint32_t> stats;
            for(uint32_t i=0; i<_capacity; i++){
                if(_a[i].empty()) continue;
                stats[_a[i].first] = 1;
            }
            for(auto [key, _]: stats){
                bool ok = false; uint32_t i;
                i = _hash(key, _capacity, _hash_seed);
                while(!_a[i].empty()) {
                    if(_a[i].first == (key)) { ok = true; break; }
                    i = (i + 1) % _capacity;
                    stats[key]++;
                }
            }
            for(uint32_t i=0; i<_capacity; i++){
                if(_a[i].empty()) {
                    std::cout << i << ": <NULL>" << std::endl;
                    continue;
                }
                std::cout << i << ": <" << _a[i].first.str() << ", " << stats[_a[i].first] << '>' << std::endl;
            }
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
    };

} // namespace pkpy