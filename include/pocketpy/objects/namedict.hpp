#pragma once

#include "pocketpy/common/config.h"
#include "pocketpy/common/str.hpp"
#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.hpp"
#include "pocketpy/objects/namedict.h"

namespace pkpy {

struct NameDict: pkpy_NameDict {
    PK_ALWAYS_PASS_BY_POINTER(NameDict)

    using Item = pair<StrName, PyVar>;

    NameDict() {
        c11_smallmap_n2v__ctor(this);
    }

    ~NameDict() {
        c11_smallmap_n2v__dtor(this);
    }

    uint16_t size() const {
        return count;
    }

    void set(StrName key, PyVar val){
        pkpy_Var* p = (pkpy_Var*)&val;
        c11_smallmap_n2v__set(this, key.index, *p);
    }

    PyVar try_get(StrName key) const{
        PyVar* p = try_get_2(key);
        return p ? *p : nullptr;
    }

    PyVar* try_get_2(StrName key) const{
        pkpy_Var* p = c11_smallmap_n2v__try_get(this, key.index);
        return p ? (PyVar*)p : nullptr;
    }

    PyVar try_get_likely_found(StrName key) const{
        return try_get(key);
    }

    PyVar* try_get_2_likely_found(StrName key) const{
        return try_get_2(key);
    }

    bool del(StrName key){
        return c11_smallmap_n2v__del(this, key.index);
    }

    bool contains(StrName key) const{
        return c11_smallmap_n2v__contains(this, key.index);
    }

    PyVar operator[] (StrName key) const{
        PyVar* val = try_get_2_likely_found(key);
        if(val == nullptr){
            PK_FATAL_ERROR("NameDict key not found: %d (%s)\n", (int)key.index, key.escape().c_str())
        }
        return *val;
    }

    void clear(){
        c11_smallmap_n2v__clear(this);
    }

    array<StrName> keys() const{
        array<StrName> retval((int)size());
        for(int i=0; i<size(); i++){
            auto it = c11__at(c11_smallmap_entry_n2v, this, i);
            retval[i] = StrName(it->key);
        }
        return retval;
    }

    array<Item> items() const{
        array<Item> retval((int)size());
        for(int i=0; i<size(); i++){
            auto it = c11__at(c11_smallmap_entry_n2v, this, i);
            PyVar* p = (PyVar*)&it->value;
            retval[i] = Item(StrName(it->key), *p);
        }
        return retval;
    }

    void apply(void (*f)(StrName, PyVar, void*), void* data){
        for(int i=0; i<size(); i++){
            auto it = c11__at(c11_smallmap_entry_n2v, this, i);
            PyVar* p = (PyVar*)&it->value;
            f(StrName(it->key), *p, data);
        }
    }
};

static_assert(sizeof(NameDict) <= 128);

}  // namespace pkpy
