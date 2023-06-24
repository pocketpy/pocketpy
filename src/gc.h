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

    int sweep(){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
#if PK_DEBUG_GC_STATS
                deleted[obj->type] += 1;
#endif
                if(_gc_on_delete) _gc_on_delete(vm, obj);
                obj->~PyObject();
                pool64.dealloc(obj);
            }
        }

        // clear _no_gc marked flag
        for(PyObject* obj: _no_gc) obj->gc.marked = false;

        int freed = gen.size() - alive.size();
        // std::cout << "GC: " << alive.size() << "/" << gen.size() << " (" << freed << " freed)" << std::endl;
        gen.clear();
        gen.swap(alive);
        return freed;
    }

    void _auto_collect(){
#if !PK_DEBUG_NO_AUTO_GC
        if(_gc_lock_counter > 0) return;
        if(gc_counter < gc_threshold) return;
        gc_counter = 0;
        collect();
        gc_threshold = gen.size() * 2;
        if(gc_threshold < kMinGCThreshold) gc_threshold = kMinGCThreshold;
#endif
    }

    int collect(){
        if(_gc_lock_counter > 0) FATAL_ERROR();
        mark();
        int freed = sweep();
        return freed;
    }

    void mark();

    ~ManagedHeap(){
        for(PyObject* obj: _no_gc) { obj->~PyObject(); pool64.dealloc(obj); }
        for(PyObject* obj: gen) { obj->~PyObject(); pool64.dealloc(obj); }
#if PK_DEBUG_GC_STATS
        for(auto& [type, count]: deleted){
            std::cout << "GC: " << obj_type_name(vm, type) << "=" << count << std::endl;
        }
#endif
    }
};

inline void FuncDecl::_gc_mark() const{
    code->_gc_mark();
    for(int i=0; i<kwargs.size(); i++) OBJ_MARK(kwargs[i].value);
}

}   // namespace pkpy
