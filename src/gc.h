#pragma once

#include "obj.h"

namespace pkpy {
    struct ManagedHeap{
        std::vector<PyObject*> heap;

        void _add(PyObject* obj){
            obj->gc.enabled = true;
            heap.push_back(obj);
        }

        void sweep(){
            std::vector<PyObject*> alive;
            for(PyObject* obj: heap){
                if(obj->gc.marked){
                    obj->gc.marked = false;
                    alive.push_back(obj);
                }else{
                    delete obj;
                }
            }
            heap.clear();
            heap.swap(alive);
        }

        void collect(VM* vm){
            std::vector<PyObject*> roots = get_roots(vm);
            for(PyObject* obj: roots) obj->mark();
            sweep();
        }

        std::vector<PyObject*> get_roots(VM* vm);
    };

}   // namespace pkpy