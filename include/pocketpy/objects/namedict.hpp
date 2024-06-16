#pragma once

#include "pocketpy/common/config.h"
#include "pocketpy/common/str.hpp"
#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.hpp"
#include "pocketpy/objects/namedict.h"

#include <string_view>

namespace pkpy {

struct NameDict: pkpy_NameDict {
    PK_ALWAYS_PASS_BY_POINTER(NameDict)

    using Item = pair<StrName, PyVar>;

    NameDict() {
        pkpy_NameDict__ctor(this);
    }

    ~NameDict() {
        pkpy_NameDict__dtor(this);
    }

    uint16_t size() const {
        return count;
    }

    void set(StrName key, PyVar val){
        PyVar* p = (PyVar*)&val;
        pkpy_NameDict__set(this, key.index, *p);
    }

    PyVar try_get(StrName key) const{
        PyVar* p = try_get_2(key);
        if(p) return *p;
        return nullptr;
    }

    PyVar* try_get_2(StrName key) const{
        PyVar* p = (PyVar*)pkpy_NameDict__try_get(this, key.index);
        return p;
    }

    PyVar try_get_likely_found(StrName key) const{
        return try_get(key);
    }

    PyVar* try_get_2_likely_found(StrName key) const{
        return try_get_2(key);
    }

    bool del(StrName key){
        return pkpy_NameDict__del(this, key.index);
    }

    bool contains(StrName key) const{
        return pkpy_NameDict__contains(this, key.index);
    }

    PyVar operator[] (StrName key) const{
        PyVar* val = try_get_2_likely_found(key);
        if(val == nullptr){
            PK_FATAL_ERROR("NameDict key not found: %d (%s)\n", (int)key.index, key.escape().c_str())
        }
        return *val;
    }

    void clear(){
        pkpy_NameDict__clear(this);
    }

    array<StrName> keys() const{
        array<StrName> retval((int)size());
        for(int i=0; i<size(); i++){
            auto it = c11__at(pkpy_NameDict_KV, this, i);
            retval[i] = StrName(it->key);
        }
        return retval;
    }

    array<Item> items() const{
        array<Item> retval((int)size());
        for(int i=0; i<size(); i++){
            auto it = c11__at(pkpy_NameDict_KV, this, i);
            PyVar* p = (PyVar*)&it->value;
            retval[i] = Item(StrName(it->key), *p);
        }
        return retval;
    }

    void apply(void (*f)(StrName, PyVar, void*), void* data){
        for(int i=0; i<size(); i++){
            auto it = c11__at(pkpy_NameDict_KV, this, i);
            PyVar* p = (PyVar*)&it->value;
            f(StrName(it->key), *p, data);
        }
    }
};

static_assert(sizeof(NameDict) <= 128);

}  // namespace pkpy
