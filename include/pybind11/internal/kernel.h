#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <optional>
#include <typeindex>
#include <stdexcept>
#include <unordered_map>

#include "pocketpy.h"
#include "type_traits.h"

namespace pkbind {

class handle;

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
        if(!indices_) { initialize(1024); }
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

template <typename T>
class lazy {
public:
    lazy(void (*init)(T&)) : init(init) {}

    operator T& () {
        if(!initialized) {
            if(init) { init(value); }
            initialized = true;
        }
        return value;
    }

    T& operator* () { return static_cast<T&>(*this); }

    void reset() { initialized = false; }

private:
    T value;
    bool initialized = false;
    void (*init)(T&) = nullptr;
};

}  // namespace pkbind
