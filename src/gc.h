#pragma once

#include "common.h"
#include "obj.h"
#include "codeobject.h"
#include "namedict.h"

namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> _no_gc;
    std::vector<PyObject*> gen;
    
    int gc_threshold = 700;
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

    ~ManagedHeap(){
        for(PyObject* obj: _no_gc) delete obj;
    }

    int sweep(VM* vm){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
                // _delete_hook(vm, obj);
                delete obj;
            }
        }

        // clear _no_gc marked flag
        for(PyObject* obj: _no_gc) obj->gc.marked = false;

        int freed = gen.size() - alive.size();
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
    }

    int collect(VM* vm){
        mark(vm);
        int freed = sweep(vm);
        // std::cout << "GC: " << freed << " objects freed" << std::endl;
        return freed;
    }

    void mark(VM* vm);
};

inline void NameDict::_mark(){
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