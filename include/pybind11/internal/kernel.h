#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <typeindex>
#include <stdexcept>
#include <unordered_map>

#include <iostream>

#include "pocketpy.h"
#include "type_traits.h"

namespace pkbind {

class handle;

/// hold the object temporarily
template <int N>
struct reg_t {
    py_Ref value;

    void operator= (py_Ref ref) & { py_setreg(N, ref); }

    operator py_Ref () & {
        assert(value && "register is not initialized");
        return value;
    }

    void operator= (handle value) &;

    operator handle () &;

    // pkpy provide user 8 registers.
    // 8th register is used for object pool, so N is limited to [0, 7).
    static_assert(N >= 0 && N <= 6, "N must be in [0, 7)");
};

struct retv_t {
    py_Ref value;

    void operator= (py_Ref ref) & { py_assign(value, ref); }

    operator py_Ref () & {
        assert(value && "return value is not initialized");
        return value;
    }

    void operator= (handle value) &;

    operator handle () &;
};

/// hold the object long time.
struct object_pool {
    inline static int cache = -1;
    inline static py_Ref pool = nullptr;
    inline static std::vector<int>* indices_ = nullptr;

    struct object_ref {
        py_Ref data;
        int index;
    };

    static void initialize(int size) noexcept {
        // use 8th register.
        pool = py_getreg(7);
        py_newtuple(pool, size);
        indices_ = new std::vector<int>(size, 0);
    }

    static void finalize() noexcept {
        delete indices_;
        indices_ = nullptr;
    }

    /// alloc an object from pool, note that the object is uninitialized.
    static object_ref alloc() {
        auto& indices = *indices_;
        if(cache != -1) {
            auto index = cache;
            cache = -1;
            indices[index] = 1;
            return {py_tuple_getitem(pool, index), index};
        }

        for(int i = 0; i < indices.size(); ++i) {
            if(indices[i] == 0) {
                indices[i] = 1;
                return {py_tuple_getitem(pool, i), i};
            }
        }

        throw std::runtime_error("object pool is full");
    }

    /// alloc an object from pool, the object is initialized with ref.
    static object_ref realloc(py_Ref ref) {
        auto result = alloc();
        py_assign(result.data, ref);
        return result;
    }

    static void inc_ref(object_ref ref) {
        if(!indices_) { return; }
        if(ref.data == py_tuple_getitem(pool, ref.index)) {
            auto& indices = *indices_;
            indices[ref.index] += 1;
        } else {
            throw std::runtime_error("object_ref is invalid");
        }
    }

    static void dec_ref(object_ref ref) {
        if(!indices_) { return; }
        if(ref.data == py_tuple_getitem(pool, ref.index)) {
            auto& indices = *indices_;
            indices[ref.index] -= 1;
            assert(indices[ref.index] >= 0 && "ref count is negative");
            if(indices[ref.index] == 0) { cache = ref.index; }
        } else {
            throw std::runtime_error("object_ref is invalid");
        }
    }
};

struct action {
    using function = void (*)();
    inline static std::vector<function> starts;

    static void initialize() noexcept {
        for(auto func: starts) {
            func();
        }
    }

    // register a function to be called at the start of the vm.
    static void register_start(function func) { starts.push_back(func); }
};

template <int N>
inline reg_t<N> reg;

inline retv_t retv;

inline std::unordered_map<std::type_index, py_Type>* m_type_map = nullptr;

}  // namespace pkbind
