#include "pocketpy/gc.h"

namespace pkpy{

    int ManagedHeap::sweep(){
        std::vector<PyObject*> alive;
        for(PyObject* obj: gen){
            if(obj->gc_marked){
                obj->gc_marked = false;
                alive.push_back(obj);
            }else{
#if PK_DEBUG_GC_STATS
                deleted[obj->type] += 1;
#endif
                if(_gc_on_delete) _gc_on_delete(vm, obj);
                obj->~PyObject();
                pool64_dealloc(obj);
            }
        }

        // clear _no_gc marked flag
        for(PyObject* obj: _no_gc) obj->gc_marked = false;

        int freed = gen.size() - alive.size();

#if PK_DEBUG_GC_STATS
        for(auto& [type, count]: deleted){
            std::cout << "GC: " << _type_name(vm, type).sv() << "=" << count << std::endl;
        }
        std::cout << "GC: " << alive.size() << "/" << gen.size() << " (" << freed << " freed)" << std::endl;
        deleted.clear();
#endif
        gen.clear();
        gen.swap(alive);
        // clean up pools
        pools_shrink_to_fit();
        return freed;
    }

    void ManagedHeap::_auto_collect(){
#if !PK_DEBUG_NO_AUTO_GC
        if(_gc_lock_counter > 0) return;
        gc_counter = 0;
        collect();
        gc_threshold = gen.size() * 2;
        if(gc_threshold < PK_GC_MIN_THRESHOLD) gc_threshold = PK_GC_MIN_THRESHOLD;
#endif
    }

    int ManagedHeap::collect(){
        PK_ASSERT(_gc_lock_counter == 0)
        mark();
        int freed = sweep();
        return freed;
    }

    ManagedHeap::~ManagedHeap(){
        for(PyObject* obj: _no_gc) { obj->~PyObject(); pool64_dealloc(obj); }
        for(PyObject* obj: gen) { obj->~PyObject(); pool64_dealloc(obj); }
    }


void FuncDecl::_gc_mark() const{
    code->_gc_mark();
    for(int i=0; i<kwargs.size(); i++) PK_OBJ_MARK(kwargs[i].value);
}

}   // namespace pkpy