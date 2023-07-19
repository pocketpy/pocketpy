#include "pocketpy/frame.h"

namespace pkpy{
    PyObject** FastLocals::try_get_name(StrName name){
        int index = varnames_inv->try_get(name);
        if(index == -1) return nullptr;
        return &a[index];
    }

    NameDict_ FastLocals::to_namedict(){
        NameDict_ dict = std::make_shared<NameDict>();
        varnames_inv->apply([&](StrName name, int index){
            PyObject* value = a[index];
            if(value != PY_NULL) dict->set(name, value);
        });
        return dict;
    }

    PyObject* Frame::f_closure_try_get(StrName name){
        if(_callable == nullptr) return nullptr;
        Function& fn = PK_OBJ_GET(Function, _callable);
        if(fn._closure == nullptr) return nullptr;
        return fn._closure->try_get(name);
    }

    Str Frame::snapshot(){
        int line = co->lines[_ip];
        return co->src->snapshot(line);
    }

    bool Frame::jump_to_exception_handler(){
        // try to find a parent try block
        int block = co->codes[_ip].block;
        while(block >= 0){
            if(co->blocks[block].type == TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return false;
        PyObject* obj = _s->popx();         // pop exception object
        // get the stack size of the try block (depth of for loops)
        int _stack_size = co->blocks[block].for_loop_depth;
        if(stack_size() < _stack_size) throw std::runtime_error("invalid stack size");
        _s->reset(actual_sp_base() + _locals.size() + _stack_size);          // rollback the stack   
        _s->push(obj);                                      // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int Frame::_exit_block(int i){
        if(co->blocks[i].type == FOR_LOOP) _s->pop();
        return co->blocks[i].parent;
    }

    void Frame::jump_abs_break(int target){
        const Bytecode& prev = co->codes[_ip];
        int i = prev.block;
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(i);
        }else{
            // BUG (solved)
            // for i in range(4):
            //     _ = 0
            // # if there is no op here, the block check will fail
            // while i: --i
            const Bytecode& next = co->codes[target];
            while(i>=0 && i!=next.block) i = _exit_block(i);
            if(i!=next.block) throw std::runtime_error("invalid jump");
        }
    }

}   // namespace pkpy