#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/pocketpy.h"
#include <stdbool.h>

void ValueStack__ctor(ValueStack* self) {
    self->sp = self->begin;
    self->end = self->begin + PK_VM_STACK_SIZE;
}

void ValueStack__clear(ValueStack* self) { self->sp = self->begin; }

py_TValue* FastLocals__try_get_by_name(py_TValue* locals, const CodeObject* co, py_Name name) {
    int index = c11_smallmap_n2i__get(&co->varnames_inv, name, -1);
    if(index == -1) return NULL;
    return &locals[index];
}

NameDict* FastLocals__to_namedict(py_TValue* locals, const CodeObject* co) {
    NameDict* dict = NameDict__new();
    c11__foreach(c11_smallmap_n2i_KV, &co->varnames_inv, entry) {
        py_TValue value = locals[entry->value];
        if(!py_isnil(&value)) { NameDict__set(dict, entry->key, value); }
    }
    return dict;
}

UnwindTarget* UnwindTarget__new(UnwindTarget* next, int iblock, int offset) {
    UnwindTarget* self = malloc(sizeof(UnwindTarget));
    self->next = next;
    self->iblock = iblock;
    self->offset = offset;
    return self;
}

void UnwindTarget__delete(UnwindTarget* self) { free(self); }

Frame* Frame__new(const CodeObject* co,
                  py_GlobalRef module,
                  py_StackRef p0,
                  py_StackRef locals,
                  bool has_function) {
    static_assert(sizeof(Frame) <= kPoolFrameBlockSize, "!(sizeof(Frame) <= kPoolFrameBlockSize)");
    Frame* self = PoolFrame_alloc();
    self->f_back = NULL;
    self->ip = (Bytecode*)co->codes.data - 1;
    self->co = co;
    self->module = module;
    self->p0 = p0;
    self->locals = locals;
    self->has_function = has_function;
    self->is_dynamic = co->src->is_dynamic;
    self->uw_list = NULL;
    return self;
}

void Frame__delete(Frame* self) {
    while(self->uw_list) {
        UnwindTarget* p = self->uw_list;
        self->uw_list = p->next;
        UnwindTarget__delete(p);
    }
    PoolFrame_dealloc(self);
}

int Frame__prepare_jump_exception_handler(Frame* self, ValueStack* _s) {
    // try to find a parent try block
    int iblock = Frame__iblock(self);
    while(iblock >= 0) {
        CodeBlock* block = c11__at(CodeBlock, &self->co->blocks, iblock);
        if(block->type == CodeBlockType_TRY_EXCEPT) break;
        iblock = block->parent;
    }
    if(iblock < 0) return -1;
    UnwindTarget* uw = Frame__find_unwind_target(self, iblock);
    _s->sp = (self->locals + uw->offset);  // unwind the stack
    return c11__at(CodeBlock, &self->co->blocks, iblock)->end;
}

void Frame__prepare_jump_break(Frame* self, ValueStack* _s, int target) {
    int iblock = Frame__iblock(self);
    if(target >= self->co->codes.length) {
        while(iblock >= 0)
            iblock = Frame__exit_block(self, _s, iblock);
    } else {
        // BUG (solved)
        // for i in range(4):
        //     _ = 0
        // # if there is no op here, the block check will fail
        // while i: --i
        int next_block = c11__at(BytecodeEx, &self->co->codes_ex, target)->iblock;
        while(iblock >= 0 && iblock != next_block)
            iblock = Frame__exit_block(self, _s, iblock);
        assert(iblock == next_block);
    }
}

int Frame__prepare_loop_break(Frame* self, ValueStack* _s) {
    int iblock = Frame__iblock(self);
    int target = c11__getitem(CodeBlock, &self->co->blocks, iblock).end;
    Frame__prepare_jump_break(self, _s, target);
    return target;
}

int Frame__exit_block(Frame* self, ValueStack* _s, int iblock) {
    CodeBlock* block = c11__at(CodeBlock, &self->co->blocks, iblock);
    if(block->type == CodeBlockType_FOR_LOOP) {
        _s->sp--;  // pop iterator
    } else if(block->type == CodeBlockType_CONTEXT_MANAGER) {
        _s->sp--;  // pop context variable
    }
    return block->parent;
}

UnwindTarget* Frame__find_unwind_target(Frame* self, int iblock) {
    UnwindTarget* uw;
    for(uw = self->uw_list; uw; uw = uw->next) {
        if(uw->iblock == iblock) return uw;
    }
    return NULL;
}

void Frame__set_unwind_target(Frame* self, py_TValue* sp) {
    int iblock = Frame__iblock(self);
    UnwindTarget* existing = Frame__find_unwind_target(self, iblock);
    if(existing) {
        existing->offset = sp - self->locals;
    } else {
        UnwindTarget* prev = self->uw_list;
        self->uw_list = UnwindTarget__new(prev, iblock, sp - self->locals);
    }
}

void Frame__gc_mark(Frame* self) {
    pk__mark_value(self->module);
    CodeObject__gc_mark(self->co);
}

py_TValue* Frame__f_closure_try_get(Frame* self, py_Name name) {
    if(!self->has_function) return NULL;
    Function* ud = py_touserdata(self->p0);
    if(ud->closure == NULL) return NULL;
    return NameDict__try_get(ud->closure, name);
}

int Frame__ip(const Frame* self) { return self->ip - (Bytecode*)self->co->codes.data; }

int Frame__lineno(const Frame* self) {
    int ip = Frame__ip(self);
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).lineno;
}

int Frame__iblock(const Frame* self) {
    int ip = Frame__ip(self);
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).iblock;
}

py_TValue* Frame__f_locals_try_get(Frame* self, py_Name name) {
    assert(!self->is_dynamic);
    return FastLocals__try_get_by_name(self->locals, self->co, name);
}