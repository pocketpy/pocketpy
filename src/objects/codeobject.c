#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/pocketpy.h"
#include <stdint.h>
#include <assert.h>

void Bytecode__set_signed_arg(Bytecode* self, int arg) {
    self->arg = (int16_t)arg;
    if((int16_t)self->arg != arg) {
        c11__abort("Bytecode__set_signed_arg(): %d is not representable in int16_t", arg);
    }
}

bool Bytecode__is_forward_jump(const Bytecode* self) {
    Opcode op = self->op;
    return (op >= OP_JUMP_FORWARD && op <= OP_LOOP_BREAK) ||
           (op == OP_FOR_ITER || op == OP_FOR_ITER_YIELD_VALUE);
}

static void FuncDecl__dtor(FuncDecl* self) {
    CodeObject__dtor(&self->code);
    c11_vector__dtor(&self->args);
    c11_vector__dtor(&self->kwargs);
    c11_smallmap_n2d__dtor(&self->kw_to_index);
}

FuncDecl_ FuncDecl__rcnew(SourceData_ src, c11_sv name) {
    FuncDecl* self = PK_MALLOC(sizeof(FuncDecl));
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

    c11_smallmap_n2d__ctor(&self->kw_to_index);
    return self;
}

bool FuncDecl__is_duplicated_arg(const FuncDecl* decl, py_Name name) {
    py_Name tmp_name;
    c11__foreach(int, &decl->args, j) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, *j);
        if(tmp_name == name) return true;
    }
    c11__foreach(FuncDeclKwArg, &decl->kwargs, kv) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, kv->index);
        if(tmp_name == name) return true;
    }
    if(decl->starred_arg != -1) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, decl->starred_arg);
        if(tmp_name == name) return true;
    }
    if(decl->starred_kwarg != -1) {
        tmp_name = c11__getitem(py_Name, &decl->code.varnames, decl->starred_kwarg);
        if(tmp_name == name) return true;
    }
    return false;
}

void FuncDecl__add_arg(FuncDecl* self, py_Name name) {
    int index = CodeObject__add_varname(&self->code, name);
    c11_vector__push(int, &self->args, index);
}

void FuncDecl__add_kwarg(FuncDecl* self, py_Name name, const py_TValue* value) {
    int index = CodeObject__add_varname(&self->code, name);
    c11_smallmap_n2d__set(&self->kw_to_index, name, index);
    FuncDeclKwArg* item = c11_vector__emplace(&self->kwargs);
    item->index = index;
    item->key = name;
    item->value = *value;
}

void FuncDecl__add_starred_arg(FuncDecl* self, py_Name name) {
    int index = CodeObject__add_varname(&self->code, name);
    self->starred_arg = index;
}

void FuncDecl__add_starred_kwarg(FuncDecl* self, py_Name name) {
    int index = CodeObject__add_varname(&self->code, name);
    self->starred_kwarg = index;
}

FuncDecl_ FuncDecl__build(c11_sv name,
                          c11_sv* args,
                          int argc,
                          c11_sv starred_arg,
                          c11_sv* kwargs,
                          int kwargc,
                          py_Ref kwdefaults,  // a tuple contains default values
                          c11_sv starred_kwarg,
                          const char* docstring) {
    SourceData_ source = SourceData__rcnew("pass", "<bind>", EXEC_MODE, false);
    FuncDecl_ decl = FuncDecl__rcnew(source, name);
    for(int i = 0; i < argc; i++) {
        FuncDecl__add_arg(decl, py_namev(args[i]));
    }
    if(starred_arg.size) { FuncDecl__add_starred_arg(decl, py_namev(starred_arg)); }
    assert(py_istype(kwdefaults, tp_tuple));
    assert(py_tuple_len(kwdefaults) == kwargc);
    for(int i = 0; i < kwargc; i++) {
        FuncDecl__add_kwarg(decl, py_namev(kwargs[i]), py_tuple_getitem(kwdefaults, i));
    }
    if(starred_kwarg.size) FuncDecl__add_starred_kwarg(decl, py_namev(starred_kwarg));
    decl->docstring = docstring;
    PK_DECREF(source);
    return decl;
}

void CodeObject__ctor(CodeObject* self, SourceData_ src, c11_sv name) {
    self->src = src;
    PK_INCREF(src);
    self->name = c11_string__new2(name.data, name.size);

    c11_vector__ctor(&self->codes, sizeof(Bytecode));
    c11_vector__ctor(&self->codes_ex, sizeof(BytecodeEx));

    c11_vector__ctor(&self->consts, sizeof(py_TValue));
    c11_vector__ctor(&self->varnames, sizeof(py_Name));
    c11_vector__ctor(&self->names, sizeof(py_Name));
    self->nlocals = 0;

    c11_smallmap_n2d__ctor(&self->varnames_inv);
    c11_smallmap_n2d__ctor(&self->names_inv);

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
    c11_vector__dtor(&self->names);

    c11_smallmap_n2d__dtor(&self->varnames_inv);
    c11_smallmap_n2d__dtor(&self->names_inv);

    c11_vector__dtor(&self->blocks);

    for(int i = 0; i < self->func_decls.length; i++) {
        FuncDecl_ decl = c11__getitem(FuncDecl_, &self->func_decls, i);
        PK_DECREF(decl);
    }
    c11_vector__dtor(&self->func_decls);
}

void Function__ctor(Function* self, FuncDecl_ decl, py_GlobalRef module, py_Ref globals) {
    PK_INCREF(decl);
    self->decl = decl;
    self->module = module;
    self->globals = globals;
    self->closure = NULL;
    self->clazz = NULL;
    self->cfunc = NULL;
}

int CodeObject__add_varname(CodeObject* self, py_Name name) {
    int index = c11_smallmap_n2d__get(&self->varnames_inv, name, -1);
    if(index >= 0) return index;
    c11_vector__push(py_Name, &self->varnames, name);
    self->nlocals++;
    index = self->varnames.length - 1;
    c11_smallmap_n2d__set(&self->varnames_inv, name, index);
    return index;
}

int CodeObject__add_name(CodeObject* self, py_Name name) {
    int index = c11_smallmap_n2d__get(&self->names_inv, name, -1);
    if(index >= 0) return index;
    c11_vector__push(py_Name, &self->names, name);
    index = self->names.length - 1;
    c11_smallmap_n2d__set(&self->names_inv, name, index);
    return index;
}

void Function__dtor(Function* self) {
    // printf("%s() in %s freed!\n", self->decl->code.name->data,
    // self->decl->code.src->filename->data);
    PK_DECREF(self->decl);
    if(self->closure) NameDict__delete(self->closure);
    memset(self, 0, sizeof(Function));
}