#pragma once

#include "obj.h"
#include "codeobject.h"
#include "namedict.h"

namespace pkpy {
struct ManagedHeap{
    std::vector<PyObject*> gen;

    template<typename T>
    PyObject* gcnew(Type type, T&& val){
        PyObject* obj = new Py_<std::decay_t<T>>(type, std::forward<T>(val));
        gen.push_back(obj);
        return obj;
    }

    template<typename T>
    PyObject* _new(Type type, T&& val){
        return gcnew<T>(type, std::forward<T>(val));
    }

    int sweep(){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc.marked){
                obj->gc.marked = false;
                alive.push_back(obj);
            }else{
                delete obj;
            }
        }
        int freed = gen.size() - alive.size();
        gen.clear();
        gen.swap(alive);
        return freed;
    }

    int collect(VM* vm){
        mark(vm);
        return sweep();
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