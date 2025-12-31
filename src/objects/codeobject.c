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

// Helper function to write binary data to a vector
static void write_bytes(c11_vector* vec, const void* data, int size) {
    c11_vector__extend(char, vec, data, size);
}

// Helper function to write an integer
static void write_int(c11_vector* vec, int value) {
    write_bytes(vec, &value, sizeof(int));
}

// Helper function to write a string
static void write_string(c11_vector* vec, c11_string* str) {
    if(str == NULL) {
        write_int(vec, -1);
    } else {
        write_int(vec, str->size);
        write_bytes(vec, str->data, str->size);
    }
}

// Helper function to check if enough bytes are available
static bool check_bounds(const char* p, const char* start, int total_size, int needed) {
    int consumed = (int)(p - start);
    return consumed >= 0 && consumed <= total_size && needed >= 0 && (total_size - consumed) >= needed;
}

// Helper function to read binary data
static const char* read_bytes(const char* p, const char* start, int total_size, void* data, int size) {
    if(!check_bounds(p, start, total_size, size)) return NULL;
    memcpy(data, p, size);
    return p + size;
}

// Helper function to read an integer
static const char* read_int(const char* p, const char* start, int total_size, int* value) {
    return read_bytes(p, start, total_size, value, sizeof(int));
}

// Helper function to read a string
static const char* read_string(const char* p, const char* start, int total_size, c11_string** str) {
    int size;
    p = read_int(p, start, total_size, &size);
    if(p == NULL) return NULL;
    if(size < -1 || size > 1048576) return NULL;  // Sanity check: max 1MB string
    if(size == -1) {
        *str = NULL;
    } else {
        if(!check_bounds(p, start, total_size, size)) return NULL;
        *str = c11_string__new2(p, size);
        p += size;
    }
    return p;
}

char* CodeObject__dumps(CodeObject* self, int* size) {
    c11_vector vec;
    c11_vector__ctor(&vec, sizeof(char));
    
    // Write a magic number for validation (PBOK in little-endian)
    int magic = 0x504B4F42;
    write_int(&vec, magic);
    
    // Write name
    write_string(&vec, self->name);
    
    // Write codes vector - check for overflow
    if(self->codes.length > 0 && self->codes.length > INT_MAX / (int)sizeof(Bytecode)) {
        c11_vector__dtor(&vec);
        return NULL;
    }
    write_int(&vec, self->codes.length);
    write_bytes(&vec, self->codes.data, self->codes.length * sizeof(Bytecode));
    
    // Write codes_ex vector - check for overflow
    if(self->codes_ex.length > 0 && self->codes_ex.length > INT_MAX / (int)sizeof(BytecodeEx)) {
        c11_vector__dtor(&vec);
        return NULL;
    }
    write_int(&vec, self->codes_ex.length);
    write_bytes(&vec, self->codes_ex.data, self->codes_ex.length * sizeof(BytecodeEx));
    
    // Write varnames vector - check for overflow
    if(self->varnames.length > 0 && self->varnames.length > INT_MAX / (int)sizeof(py_Name)) {
        c11_vector__dtor(&vec);
        return NULL;
    }
    write_int(&vec, self->varnames.length);
    write_bytes(&vec, self->varnames.data, self->varnames.length * sizeof(py_Name));
    
    // Write names vector - check for overflow
    if(self->names.length > 0 && self->names.length > INT_MAX / (int)sizeof(py_Name)) {
        c11_vector__dtor(&vec);
        return NULL;
    }
    write_int(&vec, self->names.length);
    write_bytes(&vec, self->names.data, self->names.length * sizeof(py_Name));
    
    // Write nlocals
    write_int(&vec, self->nlocals);
    
    // Write blocks vector - check for overflow
    if(self->blocks.length > 0 && self->blocks.length > INT_MAX / (int)sizeof(CodeBlock)) {
        c11_vector__dtor(&vec);
        return NULL;
    }
    write_int(&vec, self->blocks.length);
    write_bytes(&vec, self->blocks.data, self->blocks.length * sizeof(CodeBlock));
    
    // Write start_line and end_line
    write_int(&vec, self->start_line);
    write_int(&vec, self->end_line);
    
    // Return the serialized data
    *size = vec.length;
    char* result = PK_MALLOC(vec.length);
    if(result == NULL) {
        c11_vector__dtor(&vec);
        return NULL;
    }
    memcpy(result, vec.data, vec.length);
    c11_vector__dtor(&vec);
    return result;
}

bool CodeObject__loads(CodeObject* self, const char* data, int size) {
    if(data == NULL || size < sizeof(int)) return false;
    
    const char* start = data;
    const char* p = data;
    
    // Read and validate magic number
    int magic;
    p = read_int(p, start, size, &magic);
    if(p == NULL || magic != 0x504B4F42) return false;
    
    // Read name
    c11_string* name;
    p = read_string(p, start, size, &name);
    if(p == NULL) return false;
    if(self->name != NULL) {
        c11_string__delete(self->name);
    }
    self->name = name;
    
    // Read codes vector
    int codes_length;
    p = read_int(p, start, size, &codes_length);
    if(p == NULL || codes_length < 0 || codes_length > 1000000) return false;  // Sanity limit
    // Check for integer overflow before allocation
    if(codes_length > 0 && codes_length > INT_MAX / (int)sizeof(Bytecode)) return false;
    c11_vector__clear(&self->codes);
    c11_vector__reserve(&self->codes, codes_length);
    p = read_bytes(p, start, size, self->codes.data, codes_length * sizeof(Bytecode));
    if(p == NULL) return false;
    self->codes.length = codes_length;
    
    // Read codes_ex vector
    int codes_ex_length;
    p = read_int(p, start, size, &codes_ex_length);
    if(p == NULL || codes_ex_length < 0 || codes_ex_length > 1000000) return false;
    if(codes_ex_length > 0 && codes_ex_length > INT_MAX / (int)sizeof(BytecodeEx)) return false;
    c11_vector__clear(&self->codes_ex);
    c11_vector__reserve(&self->codes_ex, codes_ex_length);
    p = read_bytes(p, start, size, self->codes_ex.data, codes_ex_length * sizeof(BytecodeEx));
    if(p == NULL) return false;
    self->codes_ex.length = codes_ex_length;
    
    // Read varnames vector
    int varnames_length;
    p = read_int(p, start, size, &varnames_length);
    if(p == NULL || varnames_length < 0 || varnames_length > 100000) return false;
    if(varnames_length > 0 && varnames_length > INT_MAX / (int)sizeof(py_Name)) return false;
    c11_vector__clear(&self->varnames);
    c11_vector__reserve(&self->varnames, varnames_length);
    p = read_bytes(p, start, size, self->varnames.data, varnames_length * sizeof(py_Name));
    if(p == NULL) return false;
    self->varnames.length = varnames_length;
    
    // Read names vector
    int names_length;
    p = read_int(p, start, size, &names_length);
    if(p == NULL || names_length < 0 || names_length > 100000) return false;
    if(names_length > 0 && names_length > INT_MAX / (int)sizeof(py_Name)) return false;
    c11_vector__clear(&self->names);
    c11_vector__reserve(&self->names, names_length);
    p = read_bytes(p, start, size, self->names.data, names_length * sizeof(py_Name));
    if(p == NULL) return false;
    self->names.length = names_length;
    
    // Read nlocals
    p = read_int(p, start, size, &self->nlocals);
    if(p == NULL) return false;
    
    // Read blocks vector
    int blocks_length;
    p = read_int(p, start, size, &blocks_length);
    if(p == NULL || blocks_length < 0 || blocks_length > 100000) return false;
    if(blocks_length > 0 && blocks_length > INT_MAX / (int)sizeof(CodeBlock)) return false;
    c11_vector__clear(&self->blocks);
    c11_vector__reserve(&self->blocks, blocks_length);
    p = read_bytes(p, start, size, self->blocks.data, blocks_length * sizeof(CodeBlock));
    if(p == NULL) return false;
    self->blocks.length = blocks_length;
    
    // Read start_line and end_line
    p = read_int(p, start, size, &self->start_line);
    if(p == NULL) return false;
    p = read_int(p, start, size, &self->end_line);
    if(p == NULL) return false;
    
    // Rebuild the inverse maps
    c11_smallmap_n2d__clear(&self->varnames_inv);
    for(int i = 0; i < self->varnames.length; i++) {
        py_Name name = c11__getitem(py_Name, &self->varnames, i);
        c11_smallmap_n2d__set(&self->varnames_inv, name, i);
    }
    
    c11_smallmap_n2d__clear(&self->names_inv);
    for(int i = 0; i < self->names.length; i++) {
        py_Name name = c11__getitem(py_Name, &self->names, i);
        c11_smallmap_n2d__set(&self->names_inv, name, i);
    }
    
    return true;
}