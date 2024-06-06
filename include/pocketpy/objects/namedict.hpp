#pragma once

#include "pocketpy/common/config.h"
#include "pocketpy/common/str.hpp"
#include "pocketpy/common/utils.hpp"
#include "pocketpy/objects/object.hpp"

namespace pkpy {

struct NameDict {
    PK_ALWAYS_PASS_BY_POINTER(NameDict)

    using Item = std::pair<StrName, PyVar>;
    constexpr static uint16_t kInitialCapacity = 16;
    static_assert(is_pod_v<PyVar>);

    float _load_factor;
    uint16_t _size;

    uint16_t _capacity;
    uint16_t _critical_size;
    uint16_t _mask;

    Item* _items;

    NameDict(float load_factor = PK_INST_ATTR_LOAD_FACTOR) : _load_factor(load_factor), _size(0) {
        _set_capacity_and_alloc_items(kInitialCapacity);
    }

    ~NameDict() { std::free(_items); }

    uint16_t size() const { return _size; }

    uint16_t capacity() const { return _capacity; }

    void _set_capacity_and_alloc_items(uint16_t val);

    void set(StrName key, PyVar val);

    void _rehash_2x();

    PyVar try_get(StrName key) const;

    PyVar* try_get_2(StrName key) const;

    PyVar try_get_likely_found(StrName key) const;

    PyVar* try_get_2_likely_found(StrName key) const;

    bool del(StrName key);

    bool contains(StrName key) const;
    PyVar operator[] (StrName key) const;
    array<StrName> keys() const;
    array<Item> items() const;
    void clear();

    void apply(void (*f)(StrName, PyVar, void*), void* data);
};

static_assert(sizeof(NameDict) <= 128);
using NameDict_ = std::shared_ptr<NameDict>;

}  // namespace pkpy
