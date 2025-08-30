#include "pocketpy/pocketpy.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/vm.h"
#include <stdbool.h>

static bool disassemble(CodeObject* co) {
    c11_vector /*T=int*/ jumpTargets;
    c11_vector__ctor(&jumpTargets, sizeof(int));
    for(int i = 0; i < co->codes.length; i++) {
        Bytecode* bc = c11__at(Bytecode, &co->codes, i);
        if(Bytecode__is_forward_jump(bc)) {
            int target = (int16_t)bc->arg + i;
            c11_vector__push(int, &jumpTargets, target);
        }
    }

    c11_sbuf ss;
    c11_sbuf__ctor(&ss);

    int prev_line = -1;
    for(int i = 0; i < co->codes.length; i++) {
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
        c11_sbuf__write_char(&ss, ' ');
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
                case OP_LOAD_CONST: {
                    py_Ref value = c11__at(py_TValue, &co->consts, byte.arg);
                    if(py_repr(value)) {
                        pk_sprintf(&ss, " (%v)", py_tosv(py_retval()));
                    } else {
                        return false;
                    }
                    break;
                }
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
                case OP_DELETE_GLOBAL:
                case OP_STORE_CLASS_ATTR: {
                    py_Name name = c11__getitem(py_Name, &co->names, byte.arg);
                    pk_sprintf(&ss, " (%n)", name);
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
#define CASE_BINARY_OP(label, op, rop)                                                             \
    case label: {                                                                                  \
        pk_sprintf(&ss, " (%s)", pk_op2str(op));                                                   \
        break;                                                                                     \
    }
                    CASE_BINARY_OP(OP_BINARY_ADD, __add__, __radd__)
                    CASE_BINARY_OP(OP_BINARY_SUB, __sub__, __rsub__)
                    CASE_BINARY_OP(OP_BINARY_MUL, __mul__, __rmul__)
                    CASE_BINARY_OP(OP_BINARY_TRUEDIV, __truediv__, __rtruediv__)
                    CASE_BINARY_OP(OP_BINARY_FLOORDIV, __floordiv__, __rfloordiv__)
                    CASE_BINARY_OP(OP_BINARY_MOD, __mod__, __rmod__)
                    CASE_BINARY_OP(OP_BINARY_POW, __pow__, __rpow__)
                    CASE_BINARY_OP(OP_BINARY_LSHIFT, __lshift__, 0)
                    CASE_BINARY_OP(OP_BINARY_RSHIFT, __rshift__, 0)
                    CASE_BINARY_OP(OP_BINARY_AND, __and__, 0)
                    CASE_BINARY_OP(OP_BINARY_OR, __or__, 0)
                    CASE_BINARY_OP(OP_BINARY_XOR, __xor__, 0)
                    CASE_BINARY_OP(OP_BINARY_MATMUL, __matmul__, 0)
                    CASE_BINARY_OP(OP_COMPARE_LT, __lt__, __gt__)
                    CASE_BINARY_OP(OP_COMPARE_LE, __le__, __ge__)
                    CASE_BINARY_OP(OP_COMPARE_EQ, __eq__, __eq__)
                    CASE_BINARY_OP(OP_COMPARE_NE, __ne__, __ne__)
                    CASE_BINARY_OP(OP_COMPARE_GT, __gt__, __lt__)
                    CASE_BINARY_OP(OP_COMPARE_GE, __ge__, __le__)
#undef CASE_BINARY_OP
            }
        } while(0);

        if(i != co->codes.length - 1) c11_sbuf__write_char(&ss, '\n');
    }

    c11_string* output = c11_sbuf__submit(&ss);
    pk_current_vm->callbacks.print(output->data);
    pk_current_vm->callbacks.print("\n");
    c11_string__delete(output);
    c11_vector__dtor(&jumpTargets);
    return true;
}

static bool dis_dis(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);

    CodeObject* code = NULL;
    if(py_istype(argv, tp_function)) {
        Function* ud = py_touserdata(argv);
        code = &ud->decl->code;
    } else if(py_istype(argv, tp_code)) {
        code = py_touserdata(argv);
    } else {
        return TypeError("dis() expected a code object");
    }
    if(!disassemble(code)) return false;
    py_newnone(py_retval());
    return true;
}

void pk__add_module_dis() {
    py_Ref mod = py_newmodule("dis");

    py_bindfunc(mod, "dis", dis_dis);
}