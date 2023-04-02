#pragma once

#include "common.h"
#include "obj.h"
#include "codeobject.h"
#include "namedict.h"

/*
0: object
1: type
2: int
3: float
4: bool
5: str
6: list
7: tuple
8: slice
9: range
10: module
11: _ref
12: _star_wrapper
13: function
14: native_function
15: iterator
16: bound_method
17: super
18: Exception
19: NoneType
20: ellipsis
21: _py_op_call
22: _py_op_yield
23: re.Match
24: random.Random
25: io.FileIO
26: property
27: staticmethod
28: dict
29: set
*/

namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> _no_gc;
    std::vector<PyObject*> gen;
    
    static const int kMinGCThreshold = 700;
    int gc_threshold = kMinGCThreshold;
    int gc_counter = 0;

    template<typename T>
    PyObject* gcnew(Type type, T&& val){
        PyObject* obj = new Py_<std::decay_t<T>>(type, std::forward<T>(val));
        gen.push_back(obj);
        gc_counter++;
        return obj;
    }

    template<typename T>
    PyObject* _new(Type type, T&& val){
        PyObject* obj = new Py_<std::decay_t<T>>(type, std::forward<T>(val));
        obj->gc.enabled = false;
        _no_gc.push_back(obj);
        return obj;
    }

    inline static std::map<Type, int> deleted;

    ~ManagedHeap(){
        for(PyObject* obj: _no_gc) delete obj;
        // for(auto& [type, count]: deleted){
        //     std::cout << "GC: " << type << "=" << count << std::endl;
        // }
    }

    int sweep(VM* vm){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
                // _delete_hook(vm, obj);
                deleted[obj->type] += 1;
                delete obj;
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

    void _delete_hook(VM* vm, PyObject* obj);

    void _auto_collect(VM* vm){
        if(gc_counter < gc_threshold) return;
        gc_counter = 0;
        collect(vm);
        gc_threshold = gen.size() * 2;
        if(gc_threshold < kMinGCThreshold) gc_threshold = kMinGCThreshold;
    }

    int collect(VM* vm){
        mark(vm);
        int freed = sweep(vm);
        return freed;
    }

    void mark(VM* vm);
};

inline void NameDict::_mark() const{
    for(uint16_t i=0; i<_capacity; i++){
        if(_items[i].first.empty()) continue;
        OBJ_MARK(_items[i].second);
    }
}

template<> inline void _mark<List>(List& t){
    for(PyObject* obj: t) OBJ_MARK(obj);
}

template<> inline void _mark<Tuple>(Tuple& t){
    for(int i=0; i<t.size(); i++) OBJ_MARK(t[i]);
}

template<> inline void _mark<Function>(Function& t){
    t.code->_mark();
    t.kwargs._mark();
    if(t._module != nullptr) OBJ_MARK(t._module);
    if(t._closure != nullptr) t._closure->_mark();
}

template<> inline void _mark<BoundMethod>(BoundMethod& t){
    OBJ_MARK(t.obj);
    OBJ_MARK(t.method);
}

template<> inline void _mark<StarWrapper>(StarWrapper& t){
    OBJ_MARK(t.obj);
}

template<> inline void _mark<Super>(Super& t){
    OBJ_MARK(t.first);
}
// NOTE: std::function may capture some PyObject*, they can not be marked

}   // namespace pkpy