#include "pocketpy/interpreter/vm.h"
#include <assert.h>

void pk_print_stack(VM* self, py_Frame* frame, Bytecode byte) {
    return;
    if(frame == NULL || !self->main || py_isnil(self->main)) return;

    py_TValue* sp = self->stack.sp;
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    for(py_Ref p = self->stack.begin; p != sp; p++) {
        switch(p->type) {
            case tp_nil: c11_sbuf__write_cstr(&buf, "nil"); break;
            case tp_int: c11_sbuf__write_i64(&buf, p->_i64); break;
            case tp_float: c11_sbuf__write_f64(&buf, p->_f64, -1); break;
            case tp_bool: c11_sbuf__write_cstr(&buf, p->_bool ? "True" : "False"); break;
            case tp_NoneType: c11_sbuf__write_cstr(&buf, "None"); break;
            case tp_list: {
                pk_sprintf(&buf, "list(%d)", py_list_len(p));
                break;
            }
            case tp_tuple: {
                pk_sprintf(&buf, "tuple(%d)", py_tuple_len(p));
                break;
            }
            case tp_function: {
                Function* ud = py_touserdata(p);
                c11_sbuf__write_cstr(&buf, ud->decl->code.name->data);
                c11_sbuf__write_cstr(&buf, "()");
                break;
            }
            case tp_type: {
                pk_sprintf(&buf, "<class '%t'>", py_totype(p));
                break;
            }
            case tp_str: {
                pk_sprintf(&buf, "%q", py_tosv(p));
                break;
            }
            case tp_module: {
                py_ModuleInfo* mi = py_touserdata(p);
                pk_sprintf(&buf, "<module '%v'>", c11_string__sv(mi->path));
                break;
            }
            default: {
                pk_sprintf(&buf, "(%t)", p->type);
                break;
            }
        }
        if(p != &sp[-1]) c11_sbuf__write_cstr(&buf, ", ");
    }
    c11_string* stack_str = c11_sbuf__submit(&buf);

    printf("%s:%-3d: %-25s %-6d [%s]\n",
           frame->co->src->filename->data,
           Frame__lineno(frame),
           pk_opname(byte.op),
           byte.arg,
           stack_str->data);
    c11_string__delete(stack_str);
}

bool pk_wrapper__self(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_assign(py_retval(), argv);
    return true;
}

int py_replinput(char* buf, int max_size) {
    buf[0] = '\0';  // reset first char because we check '@' at the beginning

    int size = 0;
    bool multiline = false;
    printf(">>> ");

    while(true) {
        int c = pk_current_vm->callbacks.getchr();
        if(c == EOF) return -1;

        if(c == '\n') {
            char last = '\0';
            if(size > 0) last = buf[size - 1];
            if(multiline) {
                if(last == '\n') {
                    break;  // 2 consecutive newlines to end multiline input
                } else {
                    printf("... ");
                }
            } else {
                if(last == ':' || last == '(' || last == '[' || last == '{' || buf[0] == '@') {
                    printf("... ");
                    multiline = true;
                } else {
                    break;
                }
            }
        }

        if(size == max_size - 1) {
            buf[size] = '\0';
            return size;
        }

        buf[size++] = c;
    }

    buf[size] = '\0';
    return size;
}

py_Ref py_name2ref(py_Name name) {
    assert(name != NULL);
    CachedNames* d = &pk_current_vm->cached_names;
    py_Ref res = CachedNames__try_get(d, name);
    if(res != NULL) return res;
    // not found, create a new one
    py_StackRef tmp = py_pushtmp();
    py_newstrv(tmp, py_name2sv(name));
    CachedNames__set(d, name, tmp);
    py_pop();
    return CachedNames__try_get(d, name);
}

void PyObject__dtor(PyObject* self) {
    py_Dtor dtor = c11__getitem(TypePointer, &pk_current_vm->types, self->type).dtor;
    if(dtor) dtor(PyObject__userdata(self));
    if(self->slots == -1) {
        NameDict* dict = PyObject__dict(self);
        NameDict__dtor(dict);
    }
}