#include "pocketpy/frame.h"

namespace pkpy{
    PyVar* FastLocals::try_get_name(StrName name){
        int index = co->varnames_inv.try_get(name);
        if(index == -1) return nullptr;
        return &a[index];
    }

    NameDict_ FastLocals::to_namedict(){
        NameDict_ dict = std::make_shared<NameDict>();
        co->varnames_inv.apply([&](StrName name, int index){
            PyVar value = a[index];
            if(value != PY_NULL) dict->set(name, value);
        });
        return dict;
    }

    PyVar Frame::f_closure_try_get(StrName name){
        if(_callable == nullptr) return nullptr;
        Function& fn = PK_OBJ_GET(Function, _callable);
        if(fn._closure == nullptr) return nullptr;
        return fn._closure->try_get(name);
    }

    int Frame::prepare_jump_exception_handler(ValueStack* _s){
        // try to find a parent try block
        int block = co->lines[ip()].iblock;
        while(block >= 0){
            if(co->blocks[block].type == CodeBlockType::TRY_EXCEPT) break;
            block = co->blocks[block].parent;
        }
        if(block < 0) return -1;
        PyVar obj = _s->popx();         // pop exception object
        // get the stack size of the try block
        int _stack_size = co->blocks[block].base_stack_size;
        if(stack_size(_s) < _stack_size) throw std::runtime_error(_S("invalid state: ", stack_size(_s), '<', _stack_size).str());
        _s->reset(actual_sp_base() + _locals.size() + _stack_size);          // rollback the stack   
        _s->push(obj);                                      // push exception object
        return co->blocks[block].end;
    }

    int Frame::_exit_block(ValueStack* _s, int i){
        auto type = co->blocks[i].type;
        if(type == CodeBlockType::FOR_LOOP){
            _s->pop();  // pop the iterator
            // pop possible stack memory slots
            if(_s->top().type == kTpStackMemoryIndex){
                int count = _s->top().as<StackMemory>().count;
                PK_DEBUG_ASSERT(count < 0);
                _s->_sp += count;
                _s->_sp -= 2;   // pop header and tail
            }
        }else if(type==CodeBlockType::CONTEXT_MANAGER){
            _s->pop();
        }
        return co->blocks[i].parent;
    }

    void Frame::prepare_jump_break(ValueStack* _s, int target){
        int i = co->lines[ip()].iblock;
        if(target >= co->codes.size()){
            while(i>=0) i = _exit_block(_s, i);
        }else{
            // BUG (solved)
            // for i in range(4):
            //     _ = 0
            // # if there is no op here, the block check will fail
            // while i: --i
            int next_block = co->lines[target].iblock;
            while(i>=0 && i!=next_block) i = _exit_block(_s, i);
            if(i!=next_block) throw std::runtime_error("invalid jump");
        }
    }

}   // namespace pkpy