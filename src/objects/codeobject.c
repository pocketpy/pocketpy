#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/utils.h"
#include <stdint.h>

void Bytecode__set_signed_arg(Bytecode* self, int arg) {
    if(arg < INT16_MIN || arg > INT16_MAX) {
        PK_FATAL_ERROR("set_signed_arg: %d is out of range", arg);
    }
    self->arg = (int16_t)arg;
}

bool Bytecode__is_forward_jump(const Bytecode* self) {
    return self->op >= OP_JUMP_FORWARD && self->op <= OP_LOOP_BREAK;
}

static void FuncDecl__dtor(FuncDecl* self) {
    CodeObject__dtor(&self->code);
    c11_vector__dtor(&self->args);
    c11_vector__dtor(&self->kwargs);
    c11_smallmap_n2i__dtor(&self->kw_to_index);
}

FuncDecl_ FuncDecl__rcnew(pk_SourceData_ src, c11_sv name) {
    FuncDecl* self = malloc(sizeof(FuncDecl));
    self->rc.count = 1;
    self->rc.dtor = (void (*)(void*))FuncDecl__dtor;
    CodeObject__ctor(&self->code, src, name);

    c11_vector__ctor(&self->args, sizeof(int));
    c11_vector__ctor(&self->kwargs, sizeof(FuncDeclKwArg));

    self->starred_arg = -1;
    self->starred_kwarg = -1;
    self->nested = false;

    self->docstring = NULL;
    self->type = FuncType_UNSET;

    c11_smallmap_n2i__ctor(&self->kw_to_index);
    return self;
}

void FuncDecl__add_kwarg(FuncDecl* self, int index, uint16_t key, const py_TValue* value) {
    c11_smallmap_n2i__set(&self->kw_to_index, key, index);
    FuncDeclKwArg item = {index, key, *value};
    c11_vector__push(FuncDeclKwArg, &self->kwargs, item);
}

void CodeObject__ctor(CodeObject* self, pk_SourceData_ src, c11_sv name) {
    self->src = src;
    PK_INCREF(src);
    self->name = c11_string__new2(name.data, name.size);

    c11_vector__ctor(&self->codes, sizeof(Bytecode));
    c11_vector__ctor(&self->codes_ex, sizeof(BytecodeEx));

    c11_vector__ctor(&self->consts, sizeof(py_TValue));
    c11_vector__ctor(&self->varnames, sizeof(uint16_t));
    self->nlocals = 0;

    c11_smallmap_n2i__ctor(&self->varnames_inv);
    c11_smallmap_n2i__ctor(&self->labels);

    c11_vector__ctor(&self->blocks, sizeof(CodeBlock));
    c11_vector__ctor(&self->func_decls, sizeof(FuncDecl_));

    self->start_line = -1;
    self->end_line = -1;

    CodeBlock root_block = {CodeBlockType_NO_BLOCK, -1, 0, -1, -1};
    c11_vector__push(CodeBlock, &self->blocks, root_block);
}

void CodeObject__dtor(CodeObject* self) {
    PK_DECREF(self->src);
    c11_string__delete(self->name);

    c11_vector__dtor(&self->codes);
    c11_vector__dtor(&self->codes_ex);

    c11_vector__dtor(&self->consts);
    c11_vector__dtor(&self->varnames);

    c11_smallmap_n2i__dtor(&self->varnames_inv);
    c11_smallmap_n2i__dtor(&self->labels);

    c11_vector__dtor(&self->blocks);

    for(int i = 0; i < self->func_decls.count; i++) {
        FuncDecl_ decl = c11__getitem(FuncDecl_, &self->func_decls, i);
        PK_DECREF(decl);
    }
    c11_vector__dtor(&self->func_decls);
}

void Function__ctor(Function* self, FuncDecl_ decl, PyObject* module) {
    PK_INCREF(decl);
    self->decl = decl;
    self->module = module;
    self->clazz = NULL;
    self->closure = NULL;
    self->cfunc = NULL;
}

void Function__dtor(Function* self) {
    PK_DECREF(self->decl);
    if(self->closure) pk_NameDict__delete(self->closure);
}