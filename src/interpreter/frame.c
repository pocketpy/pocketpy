#include "pocketpy/interpreter/frame.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/codeobject.h"
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
        if(!py_isnil(&value)) NameDict__set(dict, entry->key, value);
    }
    return dict;
}

UnwindTarget* UnwindTarget__new(UnwindTarget* next, int iblock, int offset) {
    UnwindTarget* self = PK_MALLOC(sizeof(UnwindTarget));
    self->next = next;
    self->iblock = iblock;
    self->offset = offset;
    return self;
}

void UnwindTarget__delete(UnwindTarget* self) { PK_FREE(self); }

Frame* Frame__new(const CodeObject* co,
                  py_StackRef p0,
                  py_GlobalRef module,
                  py_Ref globals,
                  py_Ref locals,
                  bool is_p0_function,
                  bool is_locals_proxy) {
    assert(module->type == tp_module || module->type == tp_dict);
    Frame* self = FixedMemoryPool__alloc(&pk_current_vm->pool_frame);
    self->f_back = NULL;
    self->co = co;
    self->p0 = p0;
    self->module = module;
    self->globals = globals;
    self->locals = locals;
    self->is_p0_function = is_p0_function;
    self->is_locals_proxy = is_locals_proxy;
    self->ip = -1;
    self->uw_list = NULL;
    return self;
}

void Frame__delete(Frame* self) {
    while(self->uw_list) {
        UnwindTarget* p = self->uw_list;
        self->uw_list = p->next;
        UnwindTarget__delete(p);
    }
    FixedMemoryPool__dealloc(&pk_current_vm->pool_frame, self);
}

int Frame__prepare_jump_exception_handler(Frame* self, ValueStack* _s) {
    // try to find a parent try block
    int iblock = Frame__iblock(self);
    while(iblock >= 0) {
        CodeBlock* block = c11__at(CodeBlock, &self->co->blocks, iblock);
        if(block->type == CodeBlockType_TRY) break;
        iblock = block->parent;
    }
    if(iblock < 0) return -1;
    UnwindTarget* uw = Frame__find_unwind_target(self, iblock);
    _s->sp = (self->locals + uw->offset);  // unwind the stack
    return c11__at(CodeBlock, &self->co->blocks, iblock)->end;
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
    pk__mark_value(self->globals);
    if(self->is_locals_proxy) pk__mark_value(self->locals);
    CodeObject__gc_mark(self->co);
}

int Frame__lineno(const Frame* self) {
    int ip = self->ip;
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).lineno;
}

int Frame__iblock(const Frame* self) {
    int ip = self->ip;
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).iblock;
}

int Frame__getglobal(Frame* self, py_Name name) {
    if(self->globals->type == tp_module) {
        py_ItemRef item = py_getdict(self->globals, name);
        if(item != NULL) {
            py_assign(py_retval(), item);
            return 1;
        }
        return 0;
    } else {
        return py_dict_getitem(self->globals, py_name2ref(name));
    }
}

bool Frame__setglobal(Frame* self, py_Name name, py_TValue* val) {
    if(self->globals->type == tp_module) {
        py_setdict(self->globals, name, val);
        return true;
    } else {
        return py_dict_setitem(self->globals, py_name2ref(name), val);
    }
}

int Frame__delglobal(Frame* self, py_Name name) {
    if(self->globals->type == tp_module) {
        bool found = py_deldict(self->globals, name);
        return found ? 1 : 0;
    } else {
        return py_dict_delitem(self->globals, py_name2ref(name));
    }
}

py_StackRef Frame__getlocal_noproxy(Frame* self, py_Name name) {
    assert(!self->is_locals_proxy);
    return FastLocals__try_get_by_name(self->locals, self->co, name);
}

py_Ref Frame__getclosure(Frame* self, py_Name name) {
    if(!self->is_p0_function) return NULL;
    Function* ud = py_touserdata(self->p0);
    if(ud->closure == NULL) return NULL;
    return NameDict__try_get(ud->closure, name);
}