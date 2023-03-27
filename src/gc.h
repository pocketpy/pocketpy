#pragma once

#include "obj.h"

namespace pkpy {
    using PyVar0 = PyObject*;

    // a generational mark and sweep garbage collector
    struct GC{
        using Generation = std::vector<PyVar0>;
        static const int kTotalGen = 3;
        Generation gen[kTotalGen];

        void add(PyVar0 obj){
            if(!obj->need_gc) return;
            gen[0].push_back(obj);
        }

        void sweep(int index){
            Generation& g = gen[index];
            if(index < kTotalGen-1){
                for(int i=0; i<g.size(); i++){
                    if(g[i]->marked){
                        g[i]->marked = false;
                        gen[index+1].push_back(g[i]);
                    }else{
                        delete g[i];
                    }
                }
                g.clear();
            }else{
                Generation alive;
                // the oldest generation
                for(int i=0; i<g.size(); i++){
                    if(g[i]->marked){
                        g[i]->marked = false;
                        alive.push_back(g[i]);
                    }else{
                        delete g[i];
                    }
                }
                g = std::move(alive);
            }
        }

        void collect(int index){
            sweep(index);
        }
    };

}   // namespace pkpy