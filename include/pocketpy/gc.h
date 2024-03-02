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
        using __T = Py_<std::decay_t<T>>;
        // https://github.com/pocketpy/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64_alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<Args>(args)...);
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T, typename... Args>
    PyObject* _new(Type type, Args&&... args){
        using __T = Py_<std::decay_t<T>>;
        // https://github.com/pocketpy/pocketpy/issues/94#issuecomment-1594784476
        PyObject* obj = new(pool64_alloc<__T>()) Py_<std::decay_t<T>>(type, std::forward<Args>(args)...);
        obj->gc_enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

#if PK_DEBUG_GC_STATS
    inline static std::map<Type, int> deleted;
#endif

    int sweep();
    void _auto_collect();
    bool _should_auto_collect() const { return gc_counter >= gc_threshold; }
    int collect();
    void mark();
    ~ManagedHeap();
};

}   // namespace pkpy
