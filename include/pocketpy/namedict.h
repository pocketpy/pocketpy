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

#define PK_SMALL_NAME_DICT_CAPACITY 12
#define PK_SMALL_NAME_DICT_LOOP(B) for(int i=0; i<PK_SMALL_NAME_DICT_CAPACITY; i++) { B }

template<typename V>
struct SmallNameDict{
    using K = StrName;
    static_assert(std::is_pod_v<V>);

    bool _is_small;
    uint16_t _size;
    K _keys[PK_SMALL_NAME_DICT_CAPACITY];
    V _values[PK_SMALL_NAME_DICT_CAPACITY];

    SmallNameDict(): _is_small(true), _size(0) {}

    bool try_set(K key, V val){
        int slot = -1;

        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key){ _values[i] = val; return true; }
            if(_keys[i].empty()) slot = i;
        )

        if(slot == -1) return false;
        _keys[slot] = key;
        _values[slot] = val;
        _size++;
        return true;
    }

    V try_get(K key) const {
        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key) return _values[i];
        )
        return default_invalid_value<V>();
    }

    V* try_get_2(K key) {
        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key) return &_values[i];
        )
        return nullptr;
    }

    bool contains(K key) const {
        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key) return true;
        )
        return false;
    }

    bool del(K key){
        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            if(_keys[i] == key){ _keys[i] = StrName(); _size--; return true; }
        )
        return false;
    }

    template<typename Func>
    void apply(Func func) const {
        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            if(!_keys[i].empty()) func(_keys[i], _values[i]);
        )
    }

    void clear(){
        #pragma unroll (PK_SMALL_NAME_DICT_CAPACITY)
        PK_SMALL_NAME_DICT_LOOP(
            _keys[i] = StrName();
        )
        _size = 0;
    }

    uint16_t size() const { return _size; }
    uint16_t capacity() const { return PK_SMALL_NAME_DICT_CAPACITY; }
};

inline const uint16_t kHashSeeds[] = {9629, 43049, 13267, 59509, 39251, 1249, 27689, 9719, 19913};

inline uint16_t _hash(StrName key, uint16_t mask, uint16_t hash_seed){
    return ( (key).index * (hash_seed) >> 8 ) & (mask);
}

uint16_t _find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys);

template<typename T>
struct LargeNameDict {
    PK_ALWAYS_PASS_BY_POINTER(LargeNameDict)

    using Item = std::pair<StrName, T>;
    static constexpr uint16_t kInitialCapacity = 32;
    static_assert(is_pod<T>::value);

    bool _is_small;
    float _load_factor;
    uint16_t _size;
    uint16_t _hash_seed;

    uint16_t _capacity;
    uint16_t _critical_size;
    uint16_t _mask;

    Item* _items;

#define HASH_PROBE_1(key, ok, i)            \
ok = false;                                 \
i = _hash(key, _mask, _hash_seed);          \
while(!_items[i].first.empty()) {           \
    if(_items[i].first == (key)) { ok = true; break; }  \
    i = (i + 1) & _mask;                                \
}

#define HASH_PROBE_0 HASH_PROBE_1

    LargeNameDict(float load_factor=kInstAttrLoadFactor): _is_small(false), _load_factor(load_factor), _size(0), _hash_seed(kHashSeeds[0]) {
        _set_capacity(kInitialCapacity);
        _alloc_items();
    }

    ~LargeNameDict(){ free(_items); }

    uint16_t size() const { return _size; }
    uint16_t capacity() const { return _capacity; }

    void _set_capacity(uint16_t val){
        _capacity = val;
        _critical_size = val * _load_factor;
        _mask = val - 1;
    }

    void _alloc_items(){
        _items = (Item*)malloc(_capacity * sizeof(Item));
        memset(_items, 0, _capacity * sizeof(Item));
    }

    void set(StrName key, T val){
        bool ok; uint16_t i;
        HASH_PROBE_1(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _critical_size){
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
            _set_capacity(_capacity * 2);
        }
        _alloc_items();
        for(uint16_t i=0; i<old_capacity; i++){
            if(old_items[i].first.empty()) continue;
            bool ok; uint16_t j;
            HASH_PROBE_1(old_items[i].first, ok, j);
            if(ok) FATAL_ERROR();
            _items[j] = old_items[i];
        }
        free(old_items);
    }

    void _try_perfect_rehash(){
        _hash_seed = _find_perfect_hash_seed(_capacity, keys());
        _rehash(false); // do not resize
    }

    T try_get(StrName key) const{
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return default_invalid_value<T>();
        return _items[i].second;
    }

    T* try_get_2(StrName key) {
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return nullptr;
        return &_items[i].second;
    }

    bool contains(StrName key) const {
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        return ok;
    }

    bool del(StrName key){
        bool ok; uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) return false;
        _items[i].first = StrName();
        _items[i].second = nullptr;
        _size--;
        // tidy
        uint16_t pre_z = i;
        uint16_t z = (i + 1) & _mask;
        while(!_items[z].first.empty()){
            uint16_t h = _hash(_items[z].first, _mask, _hash_seed);
            if(h != i) break;
            std::swap(_items[pre_z], _items[z]);
            pre_z = z;
            z = (z + 1) & _mask;
        }
        return true;
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
#undef HASH_PROBE_0
#undef HASH_PROBE_1
};

template<typename V>
struct NameDictImpl{
    PK_ALWAYS_PASS_BY_POINTER(NameDictImpl)

    union{
        SmallNameDict<V> _small;
        LargeNameDict<V> _large;
    };

    NameDictImpl(): _small() {}
    NameDictImpl(float load_factor): _large(load_factor) {}

    bool is_small() const{
        const bool* p = reinterpret_cast<const bool*>(this);
        return *p;
    }

    void set(StrName key, V val){
        if(is_small()){
            bool ok = _small.try_set(key, val);
            if(!ok){
                SmallNameDict<V> copied(_small);
                // move to large name dict
                new (&_large) LargeNameDict<V>();
                copied.apply([&](StrName key, V val){
                    _large.set(key, val);
                });
                _large.set(key, val);
            }
        }else{
            _large.set(key, val);
        }
    }

    uint16_t size() const{ return is_small() ?_small.size() : _large.size(); }
    uint16_t capacity() const{ return is_small() ?_small.capacity() : _large.capacity(); }
    V try_get(StrName key) const { return is_small() ?_small.try_get(key) : _large.try_get(key); }
    V* try_get_2(StrName key) { return is_small() ?_small.try_get_2(key) : _large.try_get_2(key); }
    bool contains(StrName key) const { return is_small() ?_small.contains(key) : _large.contains(key); }
    bool del(StrName key){ return is_small() ?_small.del(key) : _large.del(key); }

    V operator[](StrName key) const {
        V val = try_get(key);
        if(val == default_invalid_value<V>()){
            throw std::runtime_error(fmt("NameDict key not found: ", key.escape()));
        }
        return val;
    }

    void clear(){
        if(is_small()) _small.clear();
        else _large.clear();
    }

    template<typename Func>
    void apply(Func func) const {
        if(is_small()) _small.apply(func);
        else _large.apply(func);
    }

    void _try_perfect_rehash(){
        if(is_small()) return;
        _large._try_perfect_rehash();
    }

    std::vector<StrName> keys() const{
        std::vector<StrName> v;
        apply([&](StrName key, V val){
            v.push_back(key);
        });
        return v;
    }

    std::vector<std::pair<StrName, V>> items() const{
        std::vector<std::pair<StrName, V>> v;
        apply([&](StrName key, V val){
            v.push_back({key, val});
        });
        return v;
    }

    ~NameDictImpl(){
        if(!is_small()) _large.~LargeNameDict<V>();
    }
};

using NameDict = NameDictImpl<PyObject*>;
using NameDict_ = std::shared_ptr<NameDict>;
using NameDictInt = NameDictImpl<int>;

static_assert(sizeof(NameDict) <= 128);

} // namespace pkpy