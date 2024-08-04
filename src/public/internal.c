#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/compiler/compiler.h"
#include <stdint.h>

pk_VM* pk_current_vm;

py_GlobalRef py_True;
py_GlobalRef py_False;
py_GlobalRef py_None;
py_GlobalRef py_NIL;

static pk_VM pk_default_vm;
static pk_VM* pk_all_vm[16];

void py_initialize() {
    pk_MemoryPools__initialize();
    py_Name__initialize();

    pk_current_vm = pk_all_vm[0] = &pk_default_vm;

    // initialize some convenient references
    static py_TValue _True, _False, _None, _NIL;
    py_newbool(&_True, true);
    py_newbool(&_False, false);
    py_newnone(&_None);
    py_newnil(&_NIL);
    py_True = &_True;
    py_False = &_False;
    py_None = &_None;
    py_NIL = &_NIL;
    pk_VM__ctor(&pk_default_vm);
}

void py_finalize() {
    for(int i = 1; i < 16; i++) {
        pk_VM* vm = pk_all_vm[i];
        if(vm) {
            pk_VM__dtor(vm);
            free(vm);
        }
    }
    pk_VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;
    py_Name__finalize();
    pk_MemoryPools__finalize();
}

void py_switchvm(int index) {
    if(index < 0 || index >= 16) c11__abort("invalid vm index");
    if(!pk_all_vm[index]) {
        pk_all_vm[index] = malloc(sizeof(pk_VM));
        pk_VM__ctor(pk_all_vm[index]);
    }
    pk_current_vm = pk_all_vm[index];
}

int py_currentvm() {
    for(int i = 0; i < 16; i++) {
        if(pk_all_vm[i] == pk_current_vm) return i;
    }
    return -1;
}

const char* pk_opname(Opcode op) {
    const static char* OP_NAMES[] = {
#define OPCODE(name) #name,
#include "pocketpy/xmacros/opcodes.h"
#undef OPCODE
    };
    return OP_NAMES[op];
}

static void disassemble(CodeObject* co) {
    c11_vector /*T=int*/ jumpTargets;
    c11_vector__ctor(&jumpTargets, sizeof(int));
    for(int i = 0; i < co->codes.count; i++) {
        Bytecode* bc = c11__at(Bytecode, &co->codes, i);
        if(Bytecode__is_forward_jump(bc)) {
            int target = (int16_t)bc->arg + i;
            c11_vector__push(int, &jumpTargets, target);
        }
    }

    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    int prev_line = -1;
    for(int i = 0; i < co->codes.count; i++) {
        Bytecode byte = c11__getitem(Bytecode, &co->codes, i);
        BytecodeEx ex = c11__getitem(BytecodeEx, &co->codes_ex, i);

        char line[8] = "";
        if(ex.lineno == prev_line) {
            // do nothing
        } else {
            snprintf(line, sizeof(line), "%d", ex.lineno);
            if(prev_line != -1) c11_sbuf__write_char(&ss, '\n');
            prev_line = ex.lineno;
        }

        char pointer[4] = "";
        c11__foreach(int, &jumpTargets, it) {
            if(*it == i) {
                snprintf(pointer, sizeof(pointer), "->");
                break;
            }
        }

        char buf[32];
        snprintf(buf, sizeof(buf), "%-8s%-3s%-3d ", line, pointer, i);
        c11_sbuf__write_cstr(&ss, buf);

        c11_sbuf__write_cstr(&ss, pk_opname(byte.op));
        c11_sbuf__write_char(&ss, ex.is_virtual ? '*' : ' ');
        int padding = 24 - strlen(pk_opname(byte.op));
        for(int j = 0; j < padding; j++)
            c11_sbuf__write_char(&ss, ' ');

        do {
            if(Bytecode__is_forward_jump(&byte)) {
                pk_sprintf(&ss, "%d (to %d)", (int16_t)byte.arg, (int16_t)byte.arg + i);
                break;
            }

            c11_sbuf__write_int(&ss, byte.arg);
            switch(byte.op) {
                case OP_LOAD_CONST:
                case OP_FORMAT_STRING:
                case OP_IMPORT_PATH: {
                    py_Ref path = c11__at(py_TValue, &co->consts, byte.arg);
                    pk_sprintf(&ss, " (%q)", py_tosv(path));
                    break;
                }
                case OP_LOAD_NAME:
                case OP_LOAD_GLOBAL:
                case OP_LOAD_NONLOCAL:
                case OP_STORE_GLOBAL:
                case OP_LOAD_ATTR:
                case OP_LOAD_METHOD:
                case OP_STORE_ATTR:
                case OP_DELETE_ATTR:
                case OP_BEGIN_CLASS:
                case OP_GOTO:
                case OP_DELETE_GLOBAL:
                case OP_STORE_CLASS_ATTR: {
                    pk_sprintf(&ss, " (%n)", byte.arg);
                    break;
                }
                case OP_LOAD_FAST:
                case OP_STORE_FAST:
                case OP_DELETE_FAST: {
                    py_Name name = c11__getitem(py_Name, &co->varnames, byte.arg);
                    pk_sprintf(&ss, " (%n)", name);
                    break;
                }
                case OP_LOAD_FUNCTION: {
                    const FuncDecl* decl = c11__getitem(FuncDecl*, &co->func_decls, byte.arg);
                    pk_sprintf(&ss, " (%s)", decl->code.name->data);
                    break;
                }
                case OP_BINARY_OP: {
                    py_Name name = byte.arg & 0xFF;
                    pk_sprintf(&ss, " (%n)", name);
                    break;
                }
            }
        } while(0);

        if(i != co->codes.count - 1) c11_sbuf__write_char(&ss, '\n');
    }

    c11_string* output = c11_sbuf__submit(&ss);
    pk_current_vm->print(output->data);
    pk_current_vm->print("\n");
    c11_string__delete(output);
    c11_vector__dtor(&jumpTargets);
}

bool py_exec(const char* source, const char* filename, enum py_CompileMode mode, py_Ref module) {
    pk_VM* vm = pk_current_vm;
    CodeObject co;
    pk_SourceData_ src = pk_SourceData__rcnew(source, filename, mode, false);
    Error* err = pk_compile(src, &co);
    if(err) {
        py_exception("SyntaxError", err->msg);
        py_BaseException__stpush(&vm->curr_exception, src, err->lineno, NULL);

        PK_DECREF(src);
        free(err);
        return false;
    }

    // disassemble(&co);
    if(!module) module = &vm->main;

    Frame* frame = Frame__new(&co, module, NULL, vm->stack.sp, vm->stack.sp, &co);
    pk_VM__push_frame(vm, frame);
    pk_FrameResult res = pk_VM__run_top_frame(vm);
    CodeObject__dtor(&co);
    PK_DECREF(src);
    if(res == RES_ERROR) return false;
    if(res == RES_RETURN) return true;
    c11__unreachedable();
}

bool py_call(py_Ref f, int argc, py_Ref argv) {
    if(f->type == tp_nativefunc) {
        return f->_cfunc(argc, argv);
    } else {
        py_push(f);
        py_pushnil();
        for(int i = 0; i < argc; i++)
            py_push(py_offset(argv, i));
        return py_vectorcall(argc, 0);
    }
}

bool py_callmethod(py_Ref self, py_Name name, int argc, py_Ref argv) { return -1; }

bool py_vectorcall(uint16_t argc, uint16_t kwargc) {
    pk_VM* vm = pk_current_vm;
    return pk_VM__vectorcall(vm, argc, kwargc, false) != RES_ERROR;
}

py_Ref py_retval() { return &pk_current_vm->last_retval; }

bool py_pushmethod(py_Name name) { return pk_pushmethod(py_peek(-1), name); }

bool pk_pushmethod(py_StackRef self, py_Name name) {
    // NOTE: `out` and `out_self` may overlap with `self`
    py_Type type;
    // handle super() proxy
    if(py_istype(self, tp_super)) {
        self = py_getslot(self, 0);
        type = *(py_Type*)py_touserdata(self);
    } else {
        type = self->type;
    }

    py_Ref cls_var = py_tpfindname(type, name);
    if(cls_var != NULL) {
        pk_current_vm->stack.sp++;
        switch(cls_var->type) {
            case tp_function:
            case tp_nativefunc: {
                py_TValue self_bak = *self;
                // `out` may overlap with `self`. If we assign `out`, `self` may be corrupted.
                self[0] = *cls_var;
                self[1] = self_bak;
                break;
            }
            case tp_staticmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = *py_NIL;
                break;
            case tp_classmethod:
                self[0] = *py_getslot(cls_var, 0);
                self[1] = c11__getitem(pk_TypeInfo, &pk_current_vm->types, type).self;
                break;
            default: c11__unreachedable();
        }
        return true;
    }
    // TODO: __getattr__ fallback
    return false;
}

py_Ref py_tpfindmagic(py_Type t, py_Name name) {
    assert(py_ismagicname(name));
    pk_TypeInfo* types = (pk_TypeInfo*)pk_current_vm->types.data;
    do {
        py_Ref f = &types[t].magic[name];
        if(!py_isnil(f)) return f;
        t = types[t].base;
    } while(t);
    return NULL;
}

py_Ref py_tpfindname(py_Type t, py_Name name) {
    pk_TypeInfo* types = (pk_TypeInfo*)pk_current_vm->types.data;
    do {
        py_Ref res = py_getdict(&types[t].self, name);
        if(res) return res;
        t = types[t].base;
    } while(t);
    return NULL;
}

py_Ref py_tpmagic(py_Type type, py_Name name) {
    assert(py_ismagicname(name));
    pk_VM* vm = pk_current_vm;
    return &c11__at(pk_TypeInfo, &vm->types, type)->magic[name];
}

py_Ref py_tpobject(py_Type type) {
    assert(type);
    pk_VM* vm = pk_current_vm;
    return &c11__at(pk_TypeInfo, &vm->types, type)->self;
}

const char* py_tpname(py_Type type) {
    if(!type) return "nil";
    pk_VM* vm = pk_current_vm;
    py_Name name = c11__at(pk_TypeInfo, &vm->types, type)->name;
    return py_name2str(name);
}

bool py_tpcall(py_Type type, int argc, py_Ref argv) {
    return py_call(py_tpobject(type), argc, argv);
}

bool pk_callmagic(py_Name name, int argc, py_Ref argv) {
    assert(argc >= 1);
    assert(py_ismagicname(name));
    py_Ref tmp = py_tpfindmagic(argv->type, name);
    if(!tmp) return AttributeError(argv, name);
    return py_call(tmp, argc, argv);
}

bool StopIteration() {
    pk_VM* vm = pk_current_vm;
    assert(!vm->is_stopiteration);  // flag is already set
    vm->is_stopiteration = true;
    return false;
}