#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/serialize.h"
#include "pocketpy/common/utils.h"

// Magic number for CodeObject serialization: "CO" = 0x434F
#define CODEOBJECT_MAGIC 0x434F
#define CODEOBJECT_VER_MAJOR 1
#define CODEOBJECT_VER_MINOR 0
#define CODEOBJECT_VER_MINOR_MIN 0

// Forward declarations
static void FuncDecl__serialize(c11_serializer* s,
                                const FuncDecl* decl,
                                const struct SourceData* parent_src);
static FuncDecl_ FuncDecl__deserialize(c11_deserializer* d, SourceData_ embedded_src);
static void CodeObject__serialize(c11_serializer* s,
                                  const CodeObject* co,
                                  const struct SourceData* parent_src);
static CodeObject CodeObject__deserialize(c11_deserializer* d, const char* filename, SourceData_ embedded_src);

// Serialize a py_TValue constant
static void TValue__serialize(c11_serializer* s, py_Ref val) {
    c11_serializer__write_type(s, val->type);
    // 1. co_consts: int | float | str
    // 2. function defaults: see `read_literal()` in compiler.c
    switch(val->type) {
        case tp_int: c11_serializer__write_i64(s, val->_i64); break;
        case tp_float: c11_serializer__write_f64(s, val->_f64); break;
        case tp_str: {
            c11_sv sv = py_tosv((py_Ref)val);
            c11_serializer__write_i32(s, sv.size);
            c11_serializer__write_bytes(s, sv.data, sv.size);
            break;
        }
        case tp_bool: {
            bool value = py_tobool(val);
            c11_serializer__write_i8(s, value ? 1 : 0);
            break;
        }
        case tp_NoneType: break;
        case tp_ellipsis: break;
        case tp_tuple: {
            int len = py_tuple_len(val);
            c11_serializer__write_i32(s, len);
            for(int i = 0; i < len; i++) {
                py_Ref item = py_tuple_getitem(val, i);
                TValue__serialize(s, item);
            }
            break;
        }
        default: c11__abort("TValue__serialize: invalid type '%s'", py_tpname(val->type));
    }
}

// Deserialize a py_TValue constant
static void TValue__deserialize(c11_deserializer* d, py_OutRef out) {
    py_Type type = c11_deserializer__read_type(d);
    switch(type) {
        case tp_int: {
            py_i64 v = c11_deserializer__read_i64(d);
            py_newint(out, v);
            break;
        }
        case tp_float: {
            py_f64 v = c11_deserializer__read_f64(d);
            py_newfloat(out, v);
            break;
        }
        case tp_str: {
            int size = c11_deserializer__read_i32(d);
            char* dst = py_newstrn(out, size);
            char* src = c11_deserializer__read_bytes(d, size);
            memcpy(dst, src, size);
            break;
        }
        case tp_bool: {
            bool v = c11_deserializer__read_i8(d) != 0;
            py_newbool(out, v);
            break;
        }
        case tp_NoneType: {
            py_newnone(out);
            break;
        }
        case tp_ellipsis: {
            py_newellipsis(out);
            break;
        }
        case tp_tuple: {
            int len = c11_deserializer__read_i32(d);
            py_newtuple(out, len);
            for(int i = 0; i < len; i++) {
                py_ItemRef item = py_tuple_getitem(out, i);
                TValue__deserialize(d, item);
            }
            break;
        }
        default:
            c11__abort("TValue__deserialize: invalid type '%s'", py_tpname(type));
    }
}

// Serialize CodeObject
static void CodeObject__serialize(c11_serializer* s,
                                  const CodeObject* co,
                                  const struct SourceData* parent_src) {
    // SourceData
    if(parent_src) {
        c11__rtassert(co->src == parent_src);
    }

    // name
    c11_serializer__write_cstr(s, co->name->data);

    // codes
    _Static_assert(sizeof(Bytecode) == sizeof(uint16_t) * 2, "");
    c11_serializer__write_i32(s, co->codes.length);
    c11_serializer__write_mark(s, '[');
    c11_serializer__write_bytes(s, co->codes.data, co->codes.length * sizeof(Bytecode));
    c11_serializer__write_mark(s, ']');

    // codes_ex
    _Static_assert(sizeof(BytecodeEx) == sizeof(int32_t) * 2, "");
    c11_serializer__write_i32(s, co->codes_ex.length);
    c11_serializer__write_mark(s, '[');
    c11_serializer__write_bytes(s, co->codes_ex.data, co->codes_ex.length * sizeof(BytecodeEx));
    c11_serializer__write_mark(s, ']');

    // consts
    c11_serializer__write_i32(s, co->consts.length);
    c11_serializer__write_mark(s, '[');
    for(int i = 0; i < co->consts.length; i++) {
        py_Ref val = c11__at(py_TValue, &co->consts, i);
        TValue__serialize(s, val);
    }
    c11_serializer__write_mark(s, ']');

    // varnames (as cstr via py_name2str)
    c11_serializer__write_i32(s, co->varnames.length);
    c11_serializer__write_mark(s, '[');
    for(int i = 0; i < co->varnames.length; i++) {
        py_Name name = c11__getitem(py_Name, &co->varnames, i);
        c11_serializer__write_cstr(s, py_name2str(name));
    }
    c11_serializer__write_mark(s, ']');

    // names (as cstr via py_name2str)
    c11_serializer__write_i32(s, co->names.length);
    c11_serializer__write_mark(s, '[');
    for(int i = 0; i < co->names.length; i++) {
        py_Name name = c11__getitem(py_Name, &co->names, i);
        c11_serializer__write_cstr(s, py_name2str(name));
    }
    c11_serializer__write_mark(s, ']');

    // nlocals
    c11_serializer__write_i32(s, co->nlocals);

    // blocks
    _Static_assert(sizeof(CodeBlock) == sizeof(int32_t) * 5, "");
    c11_serializer__write_i32(s, co->blocks.length);
    c11_serializer__write_mark(s, '[');
    c11_serializer__write_bytes(s, co->blocks.data, co->blocks.length * sizeof(CodeBlock));
    c11_serializer__write_mark(s, ']');

    // func_decls
    c11_serializer__write_i32(s, co->func_decls.length);
    c11_serializer__write_mark(s, '[');
    for(int i = 0; i < co->func_decls.length; i++) {
        const FuncDecl* decl = c11__getitem(FuncDecl_, &co->func_decls, i);
        FuncDecl__serialize(s, decl, co->src);
        c11_serializer__write_mark(s, '|');
    }
    c11_serializer__write_mark(s, ']');

    // start_line, end_line
    c11_serializer__write_i32(s, co->start_line);
    c11_serializer__write_i32(s, co->end_line);
}

// Deserialize CodeObject (initialize co before calling)
static CodeObject CodeObject__deserialize(c11_deserializer* d, const char* filename, SourceData_ embedded_src) {
    CodeObject co;

    // SourceData
    SourceData_ src;
    if(embedded_src != NULL) {
        c11__rtassert(filename == NULL);
        src = embedded_src;
        PK_INCREF(src);
    } else {
        c11__rtassert(filename != NULL);
        src = SourceData__rcnew(NULL, filename, EXEC_MODE, false);
    }

    // name
    const char* name = c11_deserializer__read_cstr(d);
    c11_sv name_sv = {name, strlen(name)};

    // Initialize the CodeObject
    CodeObject__ctor(&co, src, name_sv);
    PK_DECREF(src);  // CodeObject__ctor increments ref count
    // Clear the default root block that CodeObject__ctor adds
    c11_vector__clear(&co.blocks);

    // codes
    int codes_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    c11_vector__extend(&co.codes,
                       c11_deserializer__read_bytes(d, codes_len * sizeof(Bytecode)),
                       codes_len);
    c11_deserializer__consume_mark(d, ']');
    // codes_ex
    int codes_ex_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    c11_vector__extend(&co.codes_ex,
                       c11_deserializer__read_bytes(d, codes_ex_len * sizeof(BytecodeEx)),
                       codes_ex_len);
    c11_deserializer__consume_mark(d, ']');

    // consts
    int consts_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    for(int i = 0; i < consts_len; i++) {
        py_Ref p_val = c11_vector__emplace(&co.consts);
        TValue__deserialize(d, p_val);
    }
    c11_deserializer__consume_mark(d, ']');

    // varnames
    int varnames_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    for(int i = 0; i < varnames_len; i++) {
        const char* s = c11_deserializer__read_cstr(d);
        py_Name n = py_name(s);
        c11_vector__push(py_Name, &co.varnames, n);
        c11_smallmap_n2d__set(&co.varnames_inv, n, i);
    }
    c11_deserializer__consume_mark(d, ']');

    // names
    int names_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    for(int i = 0; i < names_len; i++) {
        const char* s = c11_deserializer__read_cstr(d);
        py_Name n = py_name(s);
        c11_vector__push(py_Name, &co.names, n);
        c11_smallmap_n2d__set(&co.names_inv, n, i);
    }
    c11_deserializer__consume_mark(d, ']');

    // nlocals
    co.nlocals = c11_deserializer__read_i32(d);

    // blocks
    int blocks_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    c11_vector__extend(&co.blocks,
                       c11_deserializer__read_bytes(d, blocks_len * sizeof(CodeBlock)),
                       blocks_len);
    c11_deserializer__consume_mark(d, ']');

    // func_decls
    int func_decls_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    for(int i = 0; i < func_decls_len; i++) {
        FuncDecl_ decl = FuncDecl__deserialize(d, src);
        c11_vector__push(FuncDecl_, &co.func_decls, decl);
        c11_deserializer__consume_mark(d, '|');
    }
    c11_deserializer__consume_mark(d, ']');

    // start_line, end_line
    co.start_line = c11_deserializer__read_i32(d);
    co.end_line = c11_deserializer__read_i32(d);

    return co;
}

// Serialize FuncDecl
static void FuncDecl__serialize(c11_serializer* s,
                                const FuncDecl* decl,
                                const struct SourceData* parent_src) {
    // CodeObject (embedded)
    c11_serializer__write_mark(s, '{');
    CodeObject__serialize(s, &decl->code, parent_src);
    c11_serializer__write_mark(s, '}');

    // args
    c11_serializer__write_i32(s, decl->args.length);
    c11_serializer__write_mark(s, '[');
    c11_serializer__write_bytes(s, decl->args.data, decl->args.length * sizeof(int32_t));
    c11_serializer__write_mark(s, ']');

    // kwargs
    c11_serializer__write_i32(s, decl->kwargs.length);
    c11_serializer__write_mark(s, '[');
    for(int i = 0; i < decl->kwargs.length; i++) {
        FuncDeclKwArg* kw = c11__at(FuncDeclKwArg, &decl->kwargs, i);
        c11_serializer__write_i32(s, kw->index);
        c11_serializer__write_cstr(s, py_name2str(kw->key));
        TValue__serialize(s, &kw->value);
    }
    c11_serializer__write_mark(s, ']');

    // starred_arg, starred_kwarg
    c11_serializer__write_i32(s, decl->starred_arg);
    c11_serializer__write_i32(s, decl->starred_kwarg);

    // nested
    c11_serializer__write_i8(s, decl->nested ? 1 : 0);

    // docstring
    int has_docstring = decl->docstring != NULL ? 1 : 0;
    c11_serializer__write_i8(s, has_docstring);
    if(has_docstring) c11_serializer__write_cstr(s, decl->docstring);

    // type
    c11_serializer__write_i8(s, (int8_t)decl->type);
}

// Deserialize FuncDecl
static FuncDecl_ FuncDecl__deserialize(c11_deserializer* d, SourceData_ embedded_src) {
    FuncDecl_ self = PK_MALLOC(sizeof(FuncDecl));
    self->rc.count = 1;
    self->rc.dtor = (void (*)(void*))FuncDecl__dtor;

    c11_vector__ctor(&self->args, sizeof(int32_t));
    c11_vector__ctor(&self->kwargs, sizeof(FuncDeclKwArg));
    c11_smallmap_n2d__ctor(&self->kw_to_index);

    // CodeObject (embedded)
    c11_deserializer__consume_mark(d, '{');
    self->code = CodeObject__deserialize(d, NULL, embedded_src);
    c11_deserializer__consume_mark(d, '}');
    
    // args
    int args_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    c11_vector__extend(&self->args,
                       c11_deserializer__read_bytes(d, args_len * sizeof(int32_t)),
                       args_len);
    c11_deserializer__consume_mark(d, ']');

    // kwargs
    int kwargs_len = c11_deserializer__read_i32(d);
    c11_deserializer__consume_mark(d, '[');
    for(int i = 0; i < kwargs_len; i++) {
        FuncDeclKwArg* kw = c11_vector__emplace(&self->kwargs);
        kw->index = c11_deserializer__read_i32(d);
        const char* key_str = c11_deserializer__read_cstr(d);
        kw->key = py_name(key_str);
        TValue__deserialize(d, &kw->value);
        c11_smallmap_n2d__set(&self->kw_to_index, kw->key, kw->index);
    }
    c11_deserializer__consume_mark(d, ']');

    // starred_arg
    self->starred_arg = c11_deserializer__read_i32(d);
    // starred_kwarg
    self->starred_kwarg = c11_deserializer__read_i32(d);

    // nested
    self->nested = c11_deserializer__read_i8(d) != 0;

    // docstring
    int has_docstring = c11_deserializer__read_i8(d);
    if(has_docstring) {
        const char* docstring = c11_deserializer__read_cstr(d);
        self->docstring = c11_strdup(docstring);
    } else {
        self->docstring = NULL;
    }

    // type
    self->type = (FuncType)c11_deserializer__read_i8(d);
    return self;
}

// Public API: Serialize CodeObject to bytes
void* CodeObject__dumps(const CodeObject* co, int* size) {
    c11_serializer s;
    c11_serializer__ctor(&s, CODEOBJECT_MAGIC, CODEOBJECT_VER_MAJOR, CODEOBJECT_VER_MINOR);
    CodeObject__serialize(&s, co, NULL);
    return c11_serializer__submit(&s, size);
}

// Public API: Deserialize CodeObject from bytes
// Returns error message or NULL on success
char* CodeObject__loads(const void* data, int size, const char* filename, CodeObject* out) {
    c11_deserializer d;
    c11_deserializer__ctor(&d, data, size);

    if(!c11_deserializer__check_header(&d,
                                       CODEOBJECT_MAGIC,
                                       CODEOBJECT_VER_MAJOR,
                                       CODEOBJECT_VER_MINOR_MIN)) {
        char* error_msg = c11_strdup(d.error_msg);
        c11_deserializer__dtor(&d);
        return error_msg;
    }

    *out = CodeObject__deserialize(&d, filename, NULL);
    c11_deserializer__dtor(&d);
    return NULL;
}

#undef CODEOBJECT_MAGIC
#undef CODEOBJECT_VER_MAJOR
#undef CODEOBJECT_VER_MINOR
#undef CODEOBJECT_VER_MINOR_MIN
