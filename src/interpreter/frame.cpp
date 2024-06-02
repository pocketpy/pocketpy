#include "pocketpy/objects/stackmemory.hpp"
#include "pocketpy/interpreter/frame.hpp"

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
            if(value) dict->set(name, value);
        });
        return dict;
    }

    PyVar* Frame::f_closure_try_get(StrName name){
        if(_callable == nullptr) return nullptr;
        Function& fn = _callable->as<Function>();
        if(fn._closure == nullptr) return nullptr;
        return fn._closure->try_get_2(name);
    }

    int Frame::prepare_jump_exception_handler(ValueStack* _s){
        // try to find a parent try block
        int i = co->lines[ip()].iblock;
        while(i >= 0){
            if(co->blocks[i].type == CodeBlockType::TRY_EXCEPT) break;
            i = co->blocks[i].parent;
        }
        if(i < 0) return -1;
        PyVar obj = _s->popx();                     // pop exception object
        UnwindTarget* uw = find_unwind_target(i);
        _s->reset(actual_sp_base() + uw->offset);   // unwind the stack
        _s->push(obj);                              // push it back
        return co->blocks[i].end;
    }

    int Frame::_exit_block(ValueStack* _s, int i){
        auto type = co->blocks[i].type;
        if(type == CodeBlockType::FOR_LOOP){
            _s->pop();  // pop the iterator
            // pop possible stack memory slots
            if(_s->top().type == kTpStackMemoryIndex){
                int count = _s->top().as<StackMemory>().count;
                assert(count < 0);
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

    void Frame::set_unwind_target(PyVar* _sp){
        int iblock = co->lines[ip()].iblock;
        UnwindTarget* existing = find_unwind_target(iblock);
        if(existing){
            existing->offset = _sp - actual_sp_base();
        }else{
            UnwindTarget* prev = _uw_list;
            _uw_list = new UnwindTarget(iblock, _sp - actual_sp_base());
            _uw_list->next = prev;
        }
    }

    UnwindTarget* Frame::find_unwind_target(int iblock){
        UnwindTarget* p;
        for(p=_uw_list; p!=nullptr; p=p->next){
            if(p->iblock == iblock) return p;
        }
        return nullptr;
    }

    Frame::~Frame(){
        while(_uw_list != nullptr){
            UnwindTarget* p = _uw_list;
            _uw_list = p->next;
            delete p;
        }
    }

    void CallStack::pop(){
        assert(!empty());
        LinkedFrame* p = _tail;
        _tail = p->f_back;
        p->~LinkedFrame();
        pool128_dealloc(p);
        --_size;
    }

    LinkedFrame* CallStack::popx(){
        assert(!empty());
        LinkedFrame* p = _tail;
        _tail = p->f_back;
        --_size;
        p->f_back = nullptr;        // unlink
        return p;
    }

    void CallStack::pushx(LinkedFrame* p){
        p->f_back = _tail;
        _tail = p;
        ++_size;
    }
}   // namespace pkpy