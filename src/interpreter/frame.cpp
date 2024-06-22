#include "pocketpy/interpreter/frame.hpp"
#include "pocketpy/common/smallmap.h"

namespace pkpy {
PyVar* FastLocals::try_get_name(StrName name) {
    int index = c11_smallmap_n2i__get(&co->varnames_inv, name.index, -1);
    if(index == -1) return nullptr;
    return &a[index];
}

NameDict* FastLocals::to_namedict() {
    NameDict* dict = new NameDict();
    for(int i=0; i<co->varnames_inv.count; i++){
        auto entry = c11__getitem(c11_smallmap_n2i_KV, &co->varnames_inv, i);
        PyVar value = a[entry.value];
        if(value) dict->set(StrName(entry.key), value);
    }
    return dict;
}

PyVar* Frame::f_closure_try_get(StrName name) {
    if(_callable == nullptr) return nullptr;
    Function& fn = _callable->as<Function>();
    if(fn._closure == nullptr) return nullptr;
    return fn._closure->try_get_2(name);
}

int Frame::prepare_jump_exception_handler(ValueStack* _s) {
    // try to find a parent try block
    int i = c11__at(BytecodeEx, &co->codes_ex, ip())->iblock;
    while(i >= 0) {
        CodeBlock* block = c11__at(CodeBlock, &co->blocks, i);
        if(block->type == CodeBlockType_TRY_EXCEPT) break;
        i = block->parent;
    }
    if(i < 0) return -1;
    PyVar obj = _s->popx();  // pop exception object
    UnwindTarget* uw = find_unwind_target(i);
    _s->reset(actual_sp_base() + uw->offset);  // unwind the stack
    _s->push(obj);                             // push it back
    return c11__at(CodeBlock, &co->blocks, i)->end;
}

int Frame::_exit_block(ValueStack* _s, int i) {
    CodeBlock* block = c11__at(CodeBlock, &co->blocks, i);
    if(block->type == CodeBlockType_FOR_LOOP) {
        _s->pop();  // pop the iterator
    } else if(block->type == CodeBlockType_CONTEXT_MANAGER) {
        _s->pop();
    }
    return block->parent;
}

void Frame::prepare_jump_break(ValueStack* _s, int target) {
    int i = c11__at(BytecodeEx, &co->codes_ex, ip())->iblock;
    if(target >= co->codes.count) {
        while(i >= 0)
            i = _exit_block(_s, i);
    } else {
        // BUG (solved)
        // for i in range(4):
        //     _ = 0
        // # if there is no op here, the block check will fail
        // while i: --i
        int next_block = c11__at(BytecodeEx, &co->codes_ex, target)->iblock;
        while(i >= 0 && i != next_block)
            i = _exit_block(_s, i);
        assert(i == next_block);
    }
}

void Frame::set_unwind_target(PyVar* _sp) {
    int iblock = c11__at(BytecodeEx, &co->codes_ex, ip())->iblock;
    UnwindTarget* existing = find_unwind_target(iblock);
    if(existing) {
        existing->offset = _sp - actual_sp_base();
    } else {
        UnwindTarget* prev = _uw_list;
        _uw_list = new UnwindTarget(iblock, _sp - actual_sp_base());
        _uw_list->next = prev;
    }
}

UnwindTarget* Frame::find_unwind_target(int iblock) {
    UnwindTarget* p;
    for(p = _uw_list; p != nullptr; p = p->next) {
        if(p->iblock == iblock) return p;
    }
    return nullptr;
}

Frame::~Frame() {
    while(_uw_list != nullptr) {
        UnwindTarget* p = _uw_list;
        _uw_list = p->next;
        delete p;
    }
}

void CallStack::pop() {
    assert(!empty());
    LinkedFrame* p = _tail;
    _tail = p->f_back;
    p->~LinkedFrame();
    PoolFrame_dealloc(p);
    --_size;
}

LinkedFrame* CallStack::popx() {
    assert(!empty());
    LinkedFrame* p = _tail;
    _tail = p->f_back;
    --_size;
    p->f_back = nullptr;  // unlink
    return p;
}

void CallStack::pushx(LinkedFrame* p) {
    p->f_back = _tail;
    _tail = p;
    ++_size;
}
}  // namespace pkpy
