#include "pocketpy/frame.h"

namespace pkpy{
    PyObject** FastLocals::try_get_name(StrName name){
        int index = co->varnames_inv.try_get(name);
        if(index == -1) return nullptr;
        return &a[index];
    }

    NameDict_ FastLocals::to_namedict(){
        NameDict_ dict = std::make_shared<NameDict>();
        co->varnames_inv.apply([&](StrName name, int index){
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

    bool Frame::jump_to_exception_handler(ValueStack* _s){
        // try to find a parent try block
        int block = co->iblocks[_ip];
        while(block >= 0){
            if(co->blocks[block].type == CodeBlockType::TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return false;
        PyObject* obj = _s->popx();         // pop exception object
        // get the stack size of the try block
        int _stack_size = co->blocks[block].base_stack_size;
        if(stack_size(_s) < _stack_size) throw std::runtime_error(_S("invalid state: ", stack_size(_s), '<', _stack_size).str());
        _s->reset(actual_sp_base() + _locals.size() + _stack_size);          // rollback the stack   
        _s->push(obj);                                      // push exception object
        _next_ip = co->blocks[block].end;
        return true;
    }

    int Frame::_exit_block(ValueStack* _s, int i){
        auto type = co->blocks[i].type;
        if(type==CodeBlockType::FOR_LOOP || type==CodeBlockType::CONTEXT_MANAGER) _s->pop();
        return co->blocks[i].parent;
    }

    void Frame::jump_abs_break(ValueStack* _s, int target){
        int i = co->iblocks[_ip];
        _next_ip = target;
        if(_next_ip >= co->codes.size()){
            while(i>=0) i = _exit_block(_s, i);
        }else{
            // BUG (solved)
            // for i in range(4):
            //     _ = 0
            // # if there is no op here, the block check will fail
            // while i: --i
            int next_block = co->iblocks[target];
            while(i>=0 && i!=next_block) i = _exit_block(_s, i);
            if(i!=next_block) throw std::runtime_error("invalid jump");
        }
    }

}   // namespace pkpy