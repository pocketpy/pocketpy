#pragma once

#include "common.h"
#include "memory.h"
#include "str.h"

namespace pkpy{

template<typename T>
constexpr T default_invalid_value(){
    if constexpr(std::is_pointer_v<T>) return nullptr;
    else if constexpr(std::is_same_v<int, T>) return -1;
    else return Discarded();
}

template<typename V>
struct SmallNameDict{
    using K = StrName;
    static_assert(std::is_pod_v<V>);

    static const int kCapacity = 12;

    int _size;
    std::pair<K, V> _items[kCapacity];

    SmallNameDict(): _size(0) {}

    void set(K key, V val){
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first == key){
                _items[i].second = val;
                return;
            }
        }
#if PK_DEBUG_EXTRA_CHECK
        if(_size == kCapacity){
            throw std::runtime_error("SmallDict: capacity exceeded");
        }
#endif
        _items[_size++] = {key, val};
    }

    bool try_set(K key, V val){
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first == key){
                _items[i].second = val;
                return true;
            }
        }
        return false;
    }

    V operator[](K key) const {
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first == key) return _items[i].second; 
        }
        throw std::out_of_range(fmt("SmallDict key not found: ", key));
    }

    V get(K key) const {
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first == key) return _items[i].second; 
        }
        return default_invalid_value<V>();
    }

    bool contains(K key) const {
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first == key) return true; 
        }
        return false;
    }

    bool del(K key){
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first == key){
                _items[i].first = StrName();
                _size--;
                return true;
            }
        }
        return false;
    }

    template<typename Func>
    void apply(Func func) const {
        for(int i=0; i<kCapacity; i++){
            if(_items[i].first) func(_items[i].first, _items[i].second);
        }
    }

    void clear(){
        for(int i=0; i<kCapacity; i++){
            _items[i].first = StrName();
        }
        _size = 0;
    }

    int size() const { return _size; }
    int capacity() const { return kCapacity; }
};

inline const uint16_t kHashSeeds[] = {9629, 43049, 13267, 59509, 39251, 1249, 27689, 9719, 19913};

inline uint16_t _hash(StrName key, uint16_t mask, uint16_t hash_seed){
    return ( (key).index * (hash_seed) >> 8 ) & (mask);
}

uint16_t _find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys);

template<typename T>
struct NameDictImpl {
    using Item = std::pair<StrName, T>;
    static constexpr uint16_t __Capacity = 8;
    // ensure the initial capacity is ok for memory pool
    static_assert(is_pod<T>::value);
    static_assert(sizeof(Item) * __Capacity <= 128);

    float _load_factor;
    uint16_t _capacity;
    uint16_t _size;
    uint16_t _hash_seed;
    uint16_t _mask;
    Item* _items;

#define HASH_PROBE_0(key, ok, i)            \
ok = false;                                 \
i = _hash(key, _mask, _hash_seed);          \
for(int _j=0; _j<_capacity; _j++) {         \
    if(!_items[i].first.empty()){           \
        if(_items[i].first == (key)) { ok = true; break; }  \
    }else{                                                  \
        if(_items[i].second == 0) break;                    \
    }                                                       \
    i = (i + 1) & _mask;                                    \
}

#define HASH_PROBE_1(key, ok, i)            \
ok = false;                                 \
i = _hash(key, _mask, _hash_seed);          \
while(!_items[i].first.empty()) {           \
    if(_items[i].first == (key)) { ok = true; break; }  \
    i = (i + 1) & _mask;                                \
}

#define NAMEDICT_ALLOC()                \
    _items = (Item*)pool128_alloc(_capacity * sizeof(Item));    \
    memset(_items, 0, _capacity * sizeof(Item));                \

    NameDictImpl(float load_factor=0.67f):
        _load_factor(load_factor), _capacity(__Capacity), _size(0), 
        _hash_seed(kHashSeeds[0]), _mask(__Capacity-1) {
        NAMEDICT_ALLOC()
    }

    NameDictImpl(const NameDictImpl& other) {
        memcpy(this, &other, sizeof(NameDictImpl));
        NAMEDICT_ALLOC()
        for(int i=0; i<_capacity; i++) _items[i] = other._items[i];
    }

    NameDictImpl& operator=(const NameDictImpl& other) {
        pool128_dealloc(_items);
        memcpy(this, &other, sizeof(NameDictImpl));
        NAMEDICT_ALLOC()
        for(int i=0; i<_capacity; i++) _items[i] = other._items[i];
        return *this;
    }
    
    ~NameDictImpl(){ pool128_dealloc(_items); }

    NameDictImpl(NameDictImpl&&) = delete;
    NameDictImpl& operator=(NameDictImpl&&) = delete;
    uint16_t size() const { return _size; }

    T operator[](StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) throw std::out_of_range(fmt("NameDict key not found: ", key));
        return _items[i].second;
    }

    void set(StrName key, T val){
        bool ok; uint16_t i;
        HASH_PROBE_1(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _capacity*_load_factor){
                _rehash(true);
                HASH_PROBE_1(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash(bool resize){
        Item* old_items = _items;
        uint16_t old_capacity = _capacity;
        if(resize){
            _capacity *= 2;
            _mask = _capacity - 1;
        }
        NAMEDICT_ALLOC()
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_items[i].first.empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE_1(old_items[i].first, ok, j);
            if(ok) FATAL_ERROR();
            _items[j] = old_items[i];
        }
        pool128_dealloc(old_items);
    }

    void _try_perfect_rehash(){
        _hash_seed = _find_perfect_hash_seed(_capacity, keys());
        _rehash(false); // do not resize
    }

    T try_get(StrName key) const{
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok){
            if constexpr(std::is_pointer_v<T>) return nullptr;
            else if constexpr(std::is_same_v<int, T>) return -1;
            else return Discarded();
        }
        return _items[i].second;
    }

    T* try_get_2(StrName key) {
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return nullptr;
        return &_items[i].second;
    }

    bool try_set(StrName key, T val){
        bool ok; uint16_t i;
        HASH_PROBE_1(key, ok, i);
        if(!ok) return false;
        _items[i].second = val;
        return true;
    }

    bool contains(StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        return ok;
    }

    void update(const NameDictImpl& other){
        for(uint16_t i=0; i<other._capacity; i++){
            auto& item = other._items[i];
            if(!item.first.empty()) set(item.first, item.second);
        }
    }

    void erase(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) throw std::out_of_range(fmt("NameDict key not found: ", key));
        _items[i].first = StrName();
        // _items[i].second = PY_DELETED_SLOT;      // do not change .second if it is not zero, it means the slot is occupied by a deleted item
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

    template<typename __Func>
    void apply(__Func func) const {
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            func(_items[i].first, _items[i].second);
        }
    }

    std::vector<StrName> keys() const {
        std::vector<StrName> v;
        for(uint16_t i=0; i<_capacity; i++){
            if(_items[i].first.empty()) continue;
            v.push_back(_items[i].first);
        }
        return v;
    }

    void clear(){
        for(uint16_t i=0; i<_capacity; i++){
            _items[i].first = StrName();
            _items[i].second = nullptr;
        }
        _size = 0;
    }
#undef HASH_PROBE
#undef NAMEDICT_ALLOC
#undef _hash
};

using NameDict = NameDictImpl<PyObject*>;
using NameDict_ = std::shared_ptr<NameDict>;
using NameDictInt = NameDictImpl<int>;

} // namespace pkpy