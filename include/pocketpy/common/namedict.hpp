#pragma once

#include "pocketpy/common/utils.hpp"
#include "pocketpy/common/str.hpp"
#include "pocketpy/common/config.h"

#include <stdexcept>

namespace pkpy {

template <typename T>
constexpr T default_invalid_value() {
    if constexpr(std::is_same_v<int, T>) {
        return -1;
    } else {
        return nullptr;
    }
}

template <typename T>
struct NameDictImpl {
    PK_ALWAYS_PASS_BY_POINTER(NameDictImpl)

    using Item = std::pair<StrName, T>;
    constexpr static uint16_t kInitialCapacity = 16;
    static_assert(is_pod_v<T>);

    float _load_factor;
    uint16_t _size;

    uint16_t _capacity;
    uint16_t _critical_size;
    uint16_t _mask;

    Item* _items;

#define HASH_PROBE_1(key, ok, i)                                                                                       \
    ok = false;                                                                                                        \
    i = key.index & _mask;                                                                                             \
    while(!_items[i].first.empty()) {                                                                                  \
        if(_items[i].first == (key)) {                                                                                 \
            ok = true;                                                                                                 \
            break;                                                                                                     \
        }                                                                                                              \
        i = (i + 1) & _mask;                                                                                           \
    }

#define HASH_PROBE_0 HASH_PROBE_1

    NameDictImpl(float load_factor = PK_INST_ATTR_LOAD_FACTOR) : _load_factor(load_factor), _size(0) {
        _set_capacity_and_alloc_items(kInitialCapacity);
    }

    ~NameDictImpl() { std::free(_items); }

    uint16_t size() const { return _size; }

    uint16_t capacity() const { return _capacity; }

    void _set_capacity_and_alloc_items(uint16_t val) {
        _capacity = val;
        _critical_size = val * _load_factor;
        _mask = val - 1;

        _items = (Item*)std::malloc(_capacity * sizeof(Item));
        std::memset(_items, 0, _capacity * sizeof(Item));
    }

    void set(StrName key, T val) {
        bool ok;
        uint16_t i;
        HASH_PROBE_1(key, ok, i);
        if(!ok) {
            _size++;
            if(_size > _critical_size) {
                _rehash_2x();
                HASH_PROBE_1(key, ok, i);
            }
            _items[i].first = key;
        }
        _items[i].second = val;
    }

    void _rehash_2x() {
        Item* old_items = _items;
        uint16_t old_capacity = _capacity;
        _set_capacity_and_alloc_items(_capacity * 2);
        for(uint16_t i = 0; i < old_capacity; i++) {
            if(old_items[i].first.empty()) { continue; }
            bool ok;
            uint16_t j;
            HASH_PROBE_1(old_items[i].first, ok, j);
            assert(!ok);
            _items[j] = old_items[i];
        }
        std::free(old_items);
    }

    T try_get(StrName key) const {
        bool ok;
        uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) { return default_invalid_value<T>(); }
        return _items[i].second;
    }

    T* try_get_2(StrName key) const {
        bool ok;
        uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) { return nullptr; }
        return &_items[i].second;
    }

    T try_get_likely_found(StrName key) const {
        uint16_t i = key.index & _mask;
        if(_items[i].first == key) { return _items[i].second; }
        i = (i + 1) & _mask;
        if(_items[i].first == key) { return _items[i].second; }
        return try_get(key);
    }

    T* try_get_2_likely_found(StrName key) const {
        uint16_t i = key.index & _mask;
        if(_items[i].first == key) { return &_items[i].second; }
        i = (i + 1) & _mask;
        if(_items[i].first == key) { return &_items[i].second; }
        return try_get_2(key);
    }

    bool del(StrName key) {
        bool ok;
        uint16_t i;
        HASH_PROBE_0(key, ok, i);
        if(!ok) { return false; }
        _items[i].first = StrName();
        _items[i].second = nullptr;
        _size--;
        // tidy
        uint16_t pre_z = i;
        uint16_t z = (i + 1) & _mask;
        while(!_items[z].first.empty()) {
            uint16_t h = _items[z].first.index & _mask;
            if(h != i) { break; }
            std::swap(_items[pre_z], _items[z]);
            pre_z = z;
            z = (z + 1) & _mask;
        }
        return true;
    }

    template <typename __Func>
    void apply(__Func func) const {
        for(uint16_t i = 0; i < _capacity; i++) {
            if(_items[i].first.empty()) { continue; }
            func(_items[i].first, _items[i].second);
        }
    }

    bool contains(StrName key) const {
        bool ok;
        uint16_t i;
        HASH_PROBE_0(key, ok, i);
        return ok;
    }

    T operator[] (StrName key) const {
        T* val = try_get_2_likely_found(key);
        if(val == nullptr) { throw std::runtime_error(_S("NameDict key not found: ", key.escape()).str()); }
        return *val;
    }

    array<StrName> keys() const {
        array<StrName> v(_size);
        int j = 0;
        for(uint16_t i = 0; i < _capacity; i++) {
            if(_items[i].first.empty()) { continue; }
            new (&v[j++]) StrName(_items[i].first);
        }
        return v;
    }

    array<Item> items() const {
        array<Item> v(_size);
        int j = 0;
        apply([&](StrName key, T val) { new (&v[j++]) Item(key, val); });
        return v;
    }

    void clear() {
        for(uint16_t i = 0; i < _capacity; i++) {
            _items[i].first = StrName();
            _items[i].second = nullptr;
        }
        _size = 0;
    }

#undef HASH_PROBE_0
#undef HASH_PROBE_1
};

}  // namespace pkpy
