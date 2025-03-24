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

void ValueStack__dtor(ValueStack* self) { self->sp = self->begin; }

void FastLocals__to_dict(py_TValue* locals, const CodeObject* co) {
    py_StackRef dict = py_pushtmp();
    py_newdict(dict);
    c11__foreach(c11_smallmap_n2i_KV, &co->varnames_inv, entry) {
        py_TValue* value = &locals[entry->value];
        if(!py_isnil(value)) {
            bool ok = py_dict_setitem(dict, py_name2ref(entry->key), value);
            assert(ok);
            (void)ok;
        }
    }
    py_assign(py_retval(), dict);
    py_pop();
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

py_Frame* Frame__new(const CodeObject* co,
                     py_StackRef p0,
                     py_GlobalRef module,
                     py_Ref globals,
                     py_Ref locals,
                     bool is_locals_special) {
    assert(module->type == tp_module);
    assert(globals->type == tp_module || globals->type == tp_dict);
    if(is_locals_special) {
        assert(locals->type == tp_nil || locals->type == tp_locals || locals->type == tp_dict);
    }
    py_Frame* self = FixedMemoryPool__alloc(&pk_current_vm->pool_frame);
    self->f_back = NULL;
    self->co = co;
    self->p0 = p0;
    self->module = module;
    self->globals = globals;
    self->locals = locals;
    self->is_locals_special = is_locals_special;
    self->ip = -1;
    self->uw_list = NULL;
    return self;
}

void Frame__delete(py_Frame* self) {
    while(self->uw_list) {
        UnwindTarget* p = self->uw_list;
        self->uw_list = p->next;
        UnwindTarget__delete(p);
    }
    FixedMemoryPool__dealloc(&pk_current_vm->pool_frame, self);
}

int Frame__prepare_jump_exception_handler(py_Frame* self, ValueStack* _s) {
    // try to find a parent try block
    int iblock = Frame__iblock(self);
    while(iblock >= 0) {
        CodeBlock* block = c11__at(CodeBlock, &self->co->blocks, iblock);
        if(block->type == CodeBlockType_TRY) break;
        iblock = block->parent;
    }
    if(iblock < 0) return -1;
    UnwindTarget* uw = Frame__find_unwind_target(self, iblock);
    _s->sp = (self->p0 + uw->offset);  // unwind the stack
    return c11__at(CodeBlock, &self->co->blocks, iblock)->end;
}

UnwindTarget* Frame__find_unwind_target(py_Frame* self, int iblock) {
    UnwindTarget* uw;
    for(uw = self->uw_list; uw; uw = uw->next) {
        if(uw->iblock == iblock) return uw;
    }
    return NULL;
}

void Frame__set_unwind_target(py_Frame* self, py_TValue* sp) {
    int iblock = Frame__iblock(self);
    assert(iblock >= 0);
    UnwindTarget* existing = Frame__find_unwind_target(self, iblock);
    if(existing) {
        existing->offset = sp - self->p0;
    } else {
        UnwindTarget* prev = self->uw_list;
        self->uw_list = UnwindTarget__new(prev, iblock, sp - self->p0);
    }
}

void Frame__gc_mark(py_Frame* self) {
    pk__mark_value(self->globals);
    if(self->is_locals_special) pk__mark_value(self->locals);
    CodeObject__gc_mark(self->co);
}

int Frame__lineno(const py_Frame* self) {
    int ip = self->ip;
    if(ip >= 0) return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).lineno;
    if(!self->is_locals_special) return self->co->start_line;
    return 0;
}

int Frame__iblock(const py_Frame* self) {
    int ip = self->ip;
    if(ip < 0) return -1;
    return c11__getitem(BytecodeEx, &self->co->codes_ex, ip).iblock;
}

int Frame__getglobal(py_Frame* self, py_Name name) {
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

bool Frame__setglobal(py_Frame* self, py_Name name, py_TValue* val) {
    if(self->globals->type == tp_module) {
        py_setdict(self->globals, name, val);
        return true;
    } else {
        return py_dict_setitem(self->globals, py_name2ref(name), val);
    }
}

int Frame__delglobal(py_Frame* self, py_Name name) {
    if(self->globals->type == tp_module) {
        bool found = py_deldict(self->globals, name);
        return found ? 1 : 0;
    } else {
        return py_dict_delitem(self->globals, py_name2ref(name));
    }
}

py_StackRef Frame__getlocal_noproxy(py_Frame* self, py_Name name) {
    assert(!self->is_locals_special);
    int index = c11_smallmap_n2i__get(&self->co->varnames_inv, name, -1);
    if(index == -1) return NULL;
    return &self->locals[index];
}

py_Ref Frame__getclosure(py_Frame* self, py_Name name) {
    if(self->is_locals_special) return NULL;
    assert(self->p0->type == tp_function);
    Function* ud = py_touserdata(self->p0);
    if(ud->closure == NULL) return NULL;
    return NameDict__try_get(ud->closure, name);
}

SourceLocation Frame__source_location(py_Frame* self) {
    SourceLocation loc;
    loc.lineno = Frame__lineno(self);
    loc.src = self->co->src;
    return loc;
}

const char* py_Frame_sourceloc(py_Frame* self, int* lineno) {
    SourceLocation loc = Frame__source_location(self);
    *lineno = loc.lineno;
    return loc.src->filename->data;
}

void py_Frame_newglobals(py_Frame* frame, py_Ref out) {
    if(!frame) {
        pk_mappingproxy__namedict(out, &pk_current_vm->main);
        return;
    }
    if(frame->globals->type == tp_module) {
        pk_mappingproxy__namedict(out, frame->globals);
    } else {
        *out = *frame->globals;  // dict
    }
}

void py_Frame_newlocals(py_Frame* frame, py_Ref out) {
    if(!frame) {
        py_newdict(out);
        return;
    }
    if(frame->is_locals_special) {
        switch(frame->locals->type) {
            case tp_locals: frame = frame->locals->_ptr; break;
            case tp_dict: *out = *frame->locals; return;
            case tp_nil: py_newdict(out); return;
            default: c11__unreachable();
        }
    }
    FastLocals__to_dict(frame->locals, frame->co);
    py_assign(out, py_retval());
}

py_StackRef py_Frame_function(py_Frame* self) {
    if(self->is_locals_special) return NULL;
    assert(self->p0->type == tp_function);
    return self->p0;
}