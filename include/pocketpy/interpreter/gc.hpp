#pragma once

#include "pocketpy/common/config.h"
#include "pocketpy/common/vector.hpp"
#include "pocketpy/common/utils.hpp"
#include "pocketpy/objects/object.hpp"

namespace pkpy {
struct ManagedHeap{
    vector<PyObject*> _no_gc;
    vector<PyObject*> gen;
    VM* vm;
    void (*_gc_on_delete)(VM*, PyObject*) = nullptr;
    void (*_gc_marker_ex)(VM*) = nullptr;

    ManagedHeap(VM* vm): vm(vm) {}
    
    int gc_threshold = PK_GC_MIN_THRESHOLD;
    int gc_counter = 0;

    /********************/
    int _gc_lock_counter = 0;
    struct ScopeLock{
        PK_ALWAYS_PASS_BY_POINTER(ScopeLock)
        
        ManagedHeap* heap;
        ScopeLock(ManagedHeap* heap): heap(heap){
            heap->_gc_lock_counter++;
        }
        ~ScopeLock(){
            heap->_gc_lock_counter--;
        }
    };

    ScopeLock gc_scope_lock(){
        return ScopeLock(this);
    }
    /********************/

    template<typename T, typename... Args>
    PyObject* gcnew(Type type, Args&&... args){
        using __T = std::decay_t<T>;
        static_assert(!is_sso_v<__T>, "gcnew cannot be used with SSO types");
        // https://github.com/pocketpy/pocketpy/issues/94#issuecomment-1594784476
        PyObject* p = new(pool128_alloc(py_sizeof<__T>)) PyObject(type);
        p->placement_new<__T>(std::forward<Args>(args)...);
        gen.push_back(p);
        gc_counter++;
        return p;
    }

    template<typename T, typename... Args>
    PyObject* _new(Type type, Args&&... args){
        using __T = std::decay_t<T>;
        static_assert(!is_sso_v<__T>);
        PyObject* p = new(pool128_alloc(py_sizeof<__T>)) PyObject(type);
        p->placement_new<__T>(std::forward<Args>(args)...);
        _no_gc.push_back(p);
        return p;
    }

    void _delete(PyObject*);

#if PK_DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    int sweep();
    void _auto_collect();
    bool _should_auto_collect() const { return gc_counter >= gc_threshold; }
    int collect();
    void mark();
};

}   // namespace pkpy
