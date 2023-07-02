#pragma once

#include "common.h"
#include "memory.h"
#include "obj.h"
#include "codeobject.h"
#include "namedict.h"

namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> _no_gc;
    std::vector<PyObject*> gen;
    VM* vm;
    void (*_gc_on_delete)(VM*, PyObject*) = nullptr;
    void (*_gc_marker_ex)(VM*) = nullptr;

    ManagedHeap(VM* vm): vm(vm) {}
    
    static const int kMinGCThreshold = 3072;
    int gc_threshold = kMinGCThreshold;
    int gc_counter = 0;

    /********************/
    int _gc_lock_counter = 0;
    struct ScopeLock{
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

    template<typename T>
    PyObject* gcnew(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
#if _WIN32
        // https://github.com/blueloveTH/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64.alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<T>(val));
#else
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
#endif
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T>
    PyObject* _new(Type type, T&& val){
        using __T = Py_<std::decay_t<T>>;
#if _WIN32
        // https://github.com/blueloveTH/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64.alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<T>(val));
#else
        PyObject* obj = new(pool64.alloc<__T>()) __T(type, std::forward<T>(val));
#endif
        obj->gc.enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

#if PK_DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    int sweep();
    void _auto_collect();
    int collect();
    void mark();
    ~ManagedHeap();
};

}   // namespace pkpy
