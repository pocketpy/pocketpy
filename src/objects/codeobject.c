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

FuncDecl_ FuncDecl__rcnew(pkpy_SourceData_ src, c11_string name){
    FuncDecl* self = malloc(sizeof(FuncDecl));
    self->rc.count = 1;
    self->rc.dtor = (void (*)(void*))FuncDecl__dtor;
    self->code = CodeObject__new(src, name);

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

void FuncDecl__dtor(FuncDecl* self){
    CodeObject__delete(self->code);
    c11_vector__dtor(&self->args);
    c11_vector__dtor(&self->kwargs);
    c11_smallmap_n2i__dtor(&self->kw_to_index);
}

void FuncDecl__add_kwarg(FuncDecl* self, int index, uint16_t key, const PyVar* value){
    c11_smallmap_n2i__set(&self->kw_to_index, key, index);
    FuncDeclKwArg item = {index, key, *value};
    c11_vector__push(FuncDeclKwArg, &self->kwargs, item);
}

CodeObject* CodeObject__new(pkpy_SourceData_ src, c11_string name){
    CodeObject* self = malloc(sizeof(CodeObject));
    self->src = src; PK_INCREF(src);
    pkpy_Str__ctor2(&self->name, name.data, name.size);

    c11_vector__ctor(&self->codes, sizeof(Bytecode));
    c11_vector__ctor(&self->codes_ex, sizeof(BytecodeEx));

    c11_vector__ctor(&self->consts, sizeof(PyVar));
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
    return self;
}

void CodeObject__delete(CodeObject* self){
    PK_DECREF(self->src);
    pkpy_Str__dtor(&self->name);
    
    c11_vector__dtor(&self->codes);
    c11_vector__dtor(&self->codes_ex);

    c11_vector__dtor(&self->consts);
    c11_vector__dtor(&self->varnames);

    c11_smallmap_n2i__dtor(&self->varnames_inv);
    c11_smallmap_n2i__dtor(&self->labels);

    c11_vector__dtor(&self->blocks);

    for(int i=0; i<self->func_decls.count; i++){
        FuncDecl_ decl = c11__getitem(FuncDecl_, &self->func_decls, i);
        PK_DECREF(decl);
    }
    c11_vector__dtor(&self->func_decls);

    free(self);
}