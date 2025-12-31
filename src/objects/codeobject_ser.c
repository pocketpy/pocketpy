#include "pocketpy/objects/codeobject.h"
#include "pocketpy/common/serialize.h"
#include "pocketpy/common/utils.h"

// Magic number for CodeObject serialization: "CO" = 0x434F
#define CODEOBJECT_MAGIC 0x434F
#define CODEOBJECT_VER_MAJOR 1
#define CODEOBJECT_VER_MINOR 0

// Forward declarations
static void FuncDecl__serialize(c11_serializer* s, const FuncDecl* decl);
static FuncDecl_ FuncDecl__deserialize(c11_deserializer* d);
static void CodeObject__serialize(c11_serializer* s, const CodeObject* co);
static void CodeObject__deserialize(c11_deserializer* d, CodeObject* co);

// Serialize a py_TValue constant
// Supported types: None, int, float, bool, str, bytes, tuple, Ellipsis
static void TValue__serialize(c11_serializer* s, const py_TValue* val) {
    c11_serializer__write_type(s, val->type);
    switch(val->type) {
        case tp_NoneType:
        case tp_ellipsis:
            // No additional data needed
            break;
        case tp_int:
            c11_serializer__write_i64(s, val->_i64);
            break;
        case tp_float:
            c11_serializer__write_f64(s, val->_f64);
            break;
        case tp_bool:
            c11_serializer__write_i8(s, val->_bool ? 1 : 0);
            break;
        case tp_str: {
            c11_sv sv = py_tosv((py_Ref)val);
            c11_serializer__write_size(s, sv.size);
            c11_serializer__write_bytes(s, sv.data, sv.size);
            break;
        }
        case tp_bytes: {
            int size;
            unsigned char* data = py_tobytes((py_Ref)val, &size);
            c11_serializer__write_size(s, size);
            c11_serializer__write_bytes(s, data, size);
            break;
        }
        case tp_tuple: {
            int len = py_tuple_len((py_Ref)val);
            c11_serializer__write_size(s, len);
            py_ObjectRef data = py_tuple_data((py_Ref)val);
            for(int i = 0; i < len; i++) {
                TValue__serialize(s, &data[i]);
            }
            break;
        }
        default:
            c11__abort("TValue__serialize: unsupported type %d", val->type);
    }
}

// Deserialize a py_TValue constant
static void TValue__deserialize(c11_deserializer* d, py_TValue* val) {
    py_Type type = c11_deserializer__read_type(d);
    switch(type) {
        case tp_NoneType:
            py_newnone(val);
            break;
        case tp_ellipsis:
            py_newellipsis(val);
            break;
        case tp_int: {
            int64_t v = c11_deserializer__read_i64(d);
            py_newint(val, v);
            break;
        }
        case tp_float: {
            double v = c11_deserializer__read_f64(d);
            py_newfloat(val, v);
            break;
        }
        case tp_bool: {
            int8_t v = c11_deserializer__read_i8(d);
            py_newbool(val, v != 0);
            break;
        }
        case tp_str: {
            int size = c11_deserializer__read_size(d);
            char* data = py_newstrn(val, size);
            memcpy(data, c11_deserializer__read_bytes(d, size), size);
            break;
        }
        case tp_bytes: {
            int size = c11_deserializer__read_size(d);
            unsigned char* data = py_newbytes(val, size);
            memcpy(data, c11_deserializer__read_bytes(d, size), size);
            break;
        }
        case tp_tuple: {
            int len = c11_deserializer__read_size(d);
            py_ObjectRef data = py_newtuple(val, len);
            for(int i = 0; i < len; i++) {
                TValue__deserialize(d, &data[i]);
            }
            break;
        }
        default:
            c11__abort("TValue__deserialize: unsupported type %d", type);
    }
}

// Serialize CodeObject
static void CodeObject__serialize(c11_serializer* s, const CodeObject* co) {
    // SourceData
    c11_serializer__write_cstr(s, co->src->filename->data);
    c11_serializer__write_cstr(s, co->src->source->data);
    c11_serializer__write_i8(s, (int8_t)co->src->mode);
    c11_serializer__write_i8(s, co->src->is_dynamic ? 1 : 0);

    // name
    c11_serializer__write_cstr(s, co->name->data);

    // codes
    c11_serializer__write_size(s, co->codes.length);
    c11_serializer__write_bytes(s, co->codes.data, co->codes.length * sizeof(Bytecode));

    // codes_ex
    c11_serializer__write_size(s, co->codes_ex.length);
    c11_serializer__write_bytes(s, co->codes_ex.data, co->codes_ex.length * sizeof(BytecodeEx));

    // consts
    c11_serializer__write_size(s, co->consts.length);
    for(int i = 0; i < co->consts.length; i++) {
        py_TValue* val = c11__at(py_TValue, &co->consts, i);
        TValue__serialize(s, val);
    }

    // varnames (as cstr via py_name2str)
    c11_serializer__write_size(s, co->varnames.length);
    for(int i = 0; i < co->varnames.length; i++) {
        py_Name name = c11__getitem(py_Name, &co->varnames, i);
        c11_serializer__write_cstr(s, py_name2str(name));
    }

    // names (as cstr via py_name2str)
    c11_serializer__write_size(s, co->names.length);
    for(int i = 0; i < co->names.length; i++) {
        py_Name name = c11__getitem(py_Name, &co->names, i);
        c11_serializer__write_cstr(s, py_name2str(name));
    }

    // nlocals
    c11_serializer__write_size(s, co->nlocals);

    // blocks
    c11_serializer__write_size(s, co->blocks.length);
    c11_serializer__write_bytes(s, co->blocks.data, co->blocks.length * sizeof(CodeBlock));

    // func_decls
    c11_serializer__write_size(s, co->func_decls.length);
    for(int i = 0; i < co->func_decls.length; i++) {
        FuncDecl_ decl = c11__getitem(FuncDecl_, &co->func_decls, i);
        FuncDecl__serialize(s, decl);
    }

    // start_line, end_line
    c11_serializer__write_size(s, co->start_line);
    c11_serializer__write_size(s, co->end_line);
}

// Deserialize CodeObject (initialize co before calling)
static void CodeObject__deserialize(c11_deserializer* d, CodeObject* co) {
    // SourceData
    const char* filename = c11_deserializer__read_cstr(d);
    const char* source = c11_deserializer__read_cstr(d);
    enum py_CompileMode mode = (enum py_CompileMode)c11_deserializer__read_i8(d);
    bool is_dynamic = c11_deserializer__read_i8(d) != 0;
    SourceData_ src = SourceData__rcnew(source, filename, mode, is_dynamic);

    // name
    const char* name = c11_deserializer__read_cstr(d);
    c11_sv name_sv = {name, strlen(name)};

    // Initialize the CodeObject
    CodeObject__ctor(co, src, name_sv);
    PK_DECREF(src);  // CodeObject__ctor increments ref count

    // Clear the default root block that CodeObject__ctor adds
    c11_vector__clear(&co->blocks);

    // codes
    int codes_len = c11_deserializer__read_size(d);
    c11_vector__reserve(&co->codes, codes_len);
    memcpy(co->codes.data, c11_deserializer__read_bytes(d, codes_len * sizeof(Bytecode)), codes_len * sizeof(Bytecode));
    co->codes.length = codes_len;

    // codes_ex
    int codes_ex_len = c11_deserializer__read_size(d);
    c11_vector__reserve(&co->codes_ex, codes_ex_len);
    memcpy(co->codes_ex.data, c11_deserializer__read_bytes(d, codes_ex_len * sizeof(BytecodeEx)), codes_ex_len * sizeof(BytecodeEx));
    co->codes_ex.length = codes_ex_len;

    // consts
    int consts_len = c11_deserializer__read_size(d);
    for(int i = 0; i < consts_len; i++) {
        py_TValue val;
        TValue__deserialize(d, &val);
        c11_vector__push(py_TValue, &co->consts, val);
    }

    // varnames
    int varnames_len = c11_deserializer__read_size(d);
    for(int i = 0; i < varnames_len; i++) {
        const char* s = c11_deserializer__read_cstr(d);
        py_Name n = py_name(s);
        c11_vector__push(py_Name, &co->varnames, n);
        c11_smallmap_n2d__set(&co->varnames_inv, n, i);
    }

    // names
    int names_len = c11_deserializer__read_size(d);
    for(int i = 0; i < names_len; i++) {
        const char* s = c11_deserializer__read_cstr(d);
        py_Name n = py_name(s);
        c11_vector__push(py_Name, &co->names, n);
        c11_smallmap_n2d__set(&co->names_inv, n, i);
    }

    // nlocals
    co->nlocals = c11_deserializer__read_size(d);

    // blocks
    int blocks_len = c11_deserializer__read_size(d);
    c11_vector__reserve(&co->blocks, blocks_len);
    memcpy(co->blocks.data, c11_deserializer__read_bytes(d, blocks_len * sizeof(CodeBlock)), blocks_len * sizeof(CodeBlock));
    co->blocks.length = blocks_len;

    // func_decls
    int func_decls_len = c11_deserializer__read_size(d);
    for(int i = 0; i < func_decls_len; i++) {
        FuncDecl_ decl = FuncDecl__deserialize(d);
        c11_vector__push(FuncDecl_, &co->func_decls, decl);
    }

    // start_line, end_line
    co->start_line = c11_deserializer__read_size(d);
    co->end_line = c11_deserializer__read_size(d);
}

// Serialize FuncDecl
static void FuncDecl__serialize(c11_serializer* s, const FuncDecl* decl) {
    // CodeObject (embedded)
    CodeObject__serialize(s, &decl->code);

    // args
    c11_serializer__write_size(s, decl->args.length);
    c11_serializer__write_bytes(s, decl->args.data, decl->args.length * sizeof(int));

    // kwargs
    c11_serializer__write_size(s, decl->kwargs.length);
    for(int i = 0; i < decl->kwargs.length; i++) {
        FuncDeclKwArg* kw = c11__at(FuncDeclKwArg, &decl->kwargs, i);
        c11_serializer__write_size(s, kw->index);
        c11_serializer__write_cstr(s, py_name2str(kw->key));
        TValue__serialize(s, &kw->value);
    }

    // starred_arg, starred_kwarg
    c11_serializer__write_size(s, decl->starred_arg);
    c11_serializer__write_size(s, decl->starred_kwarg);

    // nested
    c11_serializer__write_i8(s, decl->nested ? 1 : 0);

    // docstring
    c11_serializer__write_cstr(s, decl->docstring ? decl->docstring : "");

    // type
    c11_serializer__write_i8(s, (int8_t)decl->type);
}

// Deserialize FuncDecl
static FuncDecl_ FuncDecl__deserialize(c11_deserializer* d) {
    // We need to deserialize the CodeObject first to get src and name
    // But FuncDecl__rcnew calls CodeObject__ctor, so we deserialize differently
    
    // Allocate FuncDecl manually
    FuncDecl* self = PK_MALLOC(sizeof(FuncDecl));
    self->rc.count = 1;
    
    // Deserialize CodeObject directly into self->code
    CodeObject__deserialize(d, &self->code);
    
    // Initialize other fields
    c11_vector__ctor(&self->args, sizeof(int));
    c11_vector__ctor(&self->kwargs, sizeof(FuncDeclKwArg));
    c11_smallmap_n2d__ctor(&self->kw_to_index);

    // args
    int args_len = c11_deserializer__read_size(d);
    c11_vector__reserve(&self->args, args_len);
    memcpy(self->args.data, c11_deserializer__read_bytes(d, args_len * sizeof(int)), args_len * sizeof(int));
    self->args.length = args_len;

    // kwargs
    int kwargs_len = c11_deserializer__read_size(d);
    for(int i = 0; i < kwargs_len; i++) {
        FuncDeclKwArg kw;
        kw.index = c11_deserializer__read_size(d);
        const char* key_str = c11_deserializer__read_cstr(d);
        kw.key = py_name(key_str);
        TValue__deserialize(d, &kw.value);
        c11_vector__push(FuncDeclKwArg, &self->kwargs, kw);
        c11_smallmap_n2d__set(&self->kw_to_index, kw.key, kw.index);
    }

    // starred_arg, starred_kwarg
    self->starred_arg = c11_deserializer__read_size(d);
    self->starred_kwarg = c11_deserializer__read_size(d);

    // nested
    self->nested = c11_deserializer__read_i8(d) != 0;

    // docstring
    const char* docstring = c11_deserializer__read_cstr(d);
    self->docstring = docstring[0] ? c11_strdup(docstring) : NULL;

    // type
    self->type = (FuncType)c11_deserializer__read_i8(d);

    // Set destructor
    self->rc.dtor = NULL;  // Will be set properly when used
    
    return self;
}

// Public API: Serialize CodeObject to bytes
void* CodeObject__dumps(const CodeObject* co, int* size) {
    c11_serializer s;
    c11_serializer__ctor(&s, CODEOBJECT_MAGIC, CODEOBJECT_VER_MAJOR, CODEOBJECT_VER_MINOR);
    CodeObject__serialize(&s, co);
    return c11_serializer__submit(&s, size);
}

// Static error message buffer for CodeObject__loads
static _Thread_local char s_codeobject_error_msg[64];

// Public API: Deserialize CodeObject from bytes
// Returns error message or NULL on success
const char* CodeObject__loads(CodeObject* co, const void* data, int size) {
    c11_deserializer d;
    c11_deserializer__ctor(&d, data, size);
    
    if(!c11_deserializer__check_header(&d, CODEOBJECT_MAGIC, CODEOBJECT_VER_MAJOR, CODEOBJECT_VER_MINOR)) {
        memcpy(s_codeobject_error_msg, d.error_msg, sizeof(s_codeobject_error_msg));
        c11_deserializer__dtor(&d);
        return s_codeobject_error_msg;
    }
    
    CodeObject__deserialize(&d, co);
    c11_deserializer__dtor(&d);
    return NULL;
}
