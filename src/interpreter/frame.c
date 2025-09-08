#include "pocketpy/interpreter/frame.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"
#include <stdbool.h>
#include <assert.h>

void FastLocals__to_dict(py_TValue* locals, const CodeObject* co) {
    py_StackRef dict = py_pushtmp();
    py_newdict(dict);
    c11__foreach(c11_smallmap_n2d_KV, &co->varnames_inv, entry) {
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
    NameDict* dict = NameDict__new(PK_INST_ATTR_LOAD_FACTOR);
    c11__foreach(c11_smallmap_n2d_KV, &co->varnames_inv, entry) {
        py_Ref val = &locals[entry->value];
        if(!py_isnil(val)) NameDict__set(dict, entry->key, val);
    }
    return dict;
}

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
    c11_vector__ctor(&self->exc_stack, sizeof(FrameExcInfo));
    return self;
}

void Frame__delete(py_Frame* self) {
    c11_vector__dtor(&self->exc_stack);
    FixedMemoryPool__dealloc(&pk_current_vm->pool_frame, self);
}

int Frame__goto_exception_handler(py_Frame* self, ValueStack* value_stack, py_Ref exc) {
    FrameExcInfo* p = self->exc_stack.data;
    for(int i = self->exc_stack.length - 1; i >= 0; i--) {
        if(py_isnil(&p[i].exc)) {
            value_stack->sp = (self->p0 + p[i].offset);  // unwind the stack
            return c11__at(CodeBlock, &self->co->blocks, p[i].iblock)->end;
        } else {
            self->exc_stack.length--;
        }
    }
    return -1;
}

void Frame__begin_try(py_Frame* self, py_TValue* sp) {
    int iblock = Frame__iblock(self);
    assert(iblock >= 0);
    FrameExcInfo* info = c11_vector__emplace(&self->exc_stack);
    info->iblock = iblock;
    info->offset = (int)(sp - self->p0);
    py_newnil(&info->exc);
}

FrameExcInfo* Frame__top_exc_info(py_Frame* self) {
    if(self->exc_stack.length == 0) return NULL;
    return &c11_vector__back(FrameExcInfo, &self->exc_stack);
}

void Frame__gc_mark(py_Frame* self, c11_vector* p_stack) {
    pk__mark_value(self->globals);
    if(self->is_locals_special) pk__mark_value(self->locals);
    CodeObject__gc_mark(self->co, p_stack);
    c11__foreach(FrameExcInfo, &self->exc_stack, info) { pk__mark_value(&info->exc); }
}

int Frame__lineno(const py_Frame* self) {
    int ip = self->ip;
    if(ip >= 0) {
        BytecodeEx* ex = c11__at(BytecodeEx, &self->co->codes_ex, ip);
        return ex->lineno;
    }
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
    int index = c11_smallmap_n2d__get(&self->co->varnames_inv, name, -1);
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
