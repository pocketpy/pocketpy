#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/generator.h"
#include "pocketpy/interpreter/modules.h"
#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/common/_generated.h"
#include "pocketpy/pocketpy.h"
#include <stdbool.h>

static char* pk_default_importfile(const char* path) {
#if PK_ENABLE_OS
    FILE* f = fopen(path, "rb");
    if(f == NULL) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = PK_MALLOC(size + 1);
    size = fread(buffer, 1, size, f);
    buffer[size] = 0;
    fclose(f);
    return buffer;
#else
    return NULL;
#endif
}

static void pk_default_print(const char* data) { printf("%s", data); }

static void py_TypeInfo__ctor(py_TypeInfo* self,
                              py_Name name,
                              py_Type index,
                              py_Type base,
                              py_TypeInfo* base_ti,
                              py_TValue module) {
    memset(self, 0, sizeof(py_TypeInfo));

    self->name = name;
    self->base = base;
    self->base_ti = base_ti;

    // create type object with __dict__
    ManagedHeap* heap = &pk_current_vm->heap;
    PyObject* typeobj = ManagedHeap__new(heap, tp_type, -1, sizeof(py_Type));
    *(py_Type*)PyObject__userdata(typeobj) = index;
    self->self = (py_TValue){
        .type = typeobj->type,
        .is_ptr = true,
        ._obj = typeobj,
    };

    self->module = module;
    self->annotations = *py_NIL();
}

void VM__ctor(VM* self) {
    self->top_frame = NULL;

    ModuleDict__ctor(&self->modules, NULL, *py_NIL());
    TypeList__ctor(&self->types);

    self->builtins = *py_NIL();
    self->main = *py_NIL();

    self->callbacks.importfile = pk_default_importfile;
    self->callbacks.print = pk_default_print;
    self->callbacks.getchar = getchar;

    self->last_retval = *py_NIL();
    self->curr_exception = *py_NIL();
    self->is_signal_interrupted = false;
    self->is_curr_exc_handled = false;

    self->ctx = NULL;
    self->__curr_class = NULL;
    self->__curr_function = NULL;

    FixedMemoryPool__ctor(&self->pool_frame, sizeof(Frame), 32);

    ManagedHeap__ctor(&self->heap);
    ValueStack__ctor(&self->stack);

    /* Init Builtin Types */
    for(int i = 0; i < 128; i++) {
        char* p = py_newstrn(&self->ascii_literals[i], 1);
        *p = i;
    }
    py_newstrn(&self->ascii_literals[128], 0);

    // 0: unused
    void* placeholder = TypeList__emplace(&self->types);
    memset(placeholder, 0, sizeof(py_TypeInfo));

#define validate(t, expr)                                                                          \
    if(t != (expr)) abort()

    validate(tp_object, pk_newtype("object", 0, NULL, NULL, true, false));
    validate(tp_type, pk_newtype("type", 1, NULL, NULL, false, true));
    pk_object__register();

    validate(tp_int, pk_newtype("int", tp_object, NULL, NULL, false, true));
    validate(tp_float, pk_newtype("float", tp_object, NULL, NULL, false, true));
    validate(tp_bool, pk_newtype("bool", tp_object, NULL, NULL, false, true));
    pk_number__register();

    validate(tp_str, pk_str__register());
    validate(tp_str_iterator, pk_str_iterator__register());

    validate(tp_list, pk_list__register());
    validate(tp_tuple, pk_tuple__register());
    validate(tp_array_iterator, pk_array_iterator__register());

    validate(tp_slice, pk_slice__register());
    validate(tp_range, pk_range__register());
    validate(tp_range_iterator, pk_range_iterator__register());
    validate(tp_module, pk_newtype("module", tp_object, NULL, NULL, false, true));

    validate(tp_function, pk_function__register());
    validate(tp_nativefunc, pk_nativefunc__register());
    validate(tp_boundmethod, pk_boundmethod__register());

    validate(tp_super, pk_super__register());
    validate(tp_BaseException, pk_BaseException__register());
    validate(tp_Exception, pk_Exception__register());
    validate(tp_bytes, pk_bytes__register());
    validate(tp_namedict, pk_namedict__register());
    validate(tp_locals, pk_locals__register());
    validate(tp_code, pk_code__register());

    validate(tp_dict, pk_dict__register());
    validate(tp_dict_items, pk_dict_items__register());

    validate(tp_property, pk_property__register());
    validate(tp_star_wrapper, pk_newtype("star_wrapper", tp_object, NULL, NULL, false, true));

    validate(tp_staticmethod, pk_staticmethod__register());
    validate(tp_classmethod, pk_classmethod__register());

    validate(tp_NoneType, pk_newtype("NoneType", tp_object, NULL, NULL, false, true));
    validate(tp_NotImplementedType,
             pk_newtype("NotImplementedType", tp_object, NULL, NULL, false, true));
    validate(tp_ellipsis, pk_newtype("ellipsis", tp_object, NULL, NULL, false, true));
    validate(tp_generator, pk_generator__register());

    self->builtins = pk_builtins__register();

    // inject some builtin expections
#define INJECT_BUILTIN_EXC(name, TBase)                                                            \
    do {                                                                                           \
        py_Type type = pk_newtype(#name, TBase, &self->builtins, NULL, false, true);               \
        py_setdict(&self->builtins, py_name(#name), py_tpobject(type));                            \
        validate(tp_##name, type);                                                                 \
    } while(0)

    INJECT_BUILTIN_EXC(SystemExit, tp_BaseException);
    INJECT_BUILTIN_EXC(KeyboardInterrupt, tp_BaseException);

    validate(tp_StopIteration, pk_StopIteration__register());
    py_setdict(&self->builtins, py_name("StopIteration"), py_tpobject(tp_StopIteration));

    INJECT_BUILTIN_EXC(SyntaxError, tp_Exception);
    INJECT_BUILTIN_EXC(StackOverflowError, tp_Exception);
    INJECT_BUILTIN_EXC(OSError, tp_Exception);
    INJECT_BUILTIN_EXC(NotImplementedError, tp_Exception);
    INJECT_BUILTIN_EXC(TypeError, tp_Exception);
    INJECT_BUILTIN_EXC(IndexError, tp_Exception);
    INJECT_BUILTIN_EXC(ValueError, tp_Exception);
    INJECT_BUILTIN_EXC(RuntimeError, tp_Exception);
    INJECT_BUILTIN_EXC(ZeroDivisionError, tp_Exception);
    INJECT_BUILTIN_EXC(NameError, tp_Exception);
    INJECT_BUILTIN_EXC(UnboundLocalError, tp_Exception);
    INJECT_BUILTIN_EXC(AttributeError, tp_Exception);
    INJECT_BUILTIN_EXC(ImportError, tp_Exception);
    INJECT_BUILTIN_EXC(AssertionError, tp_Exception);
    INJECT_BUILTIN_EXC(KeyError, tp_Exception);

#undef INJECT_BUILTIN_EXC
#undef validate

    /* Setup Public Builtin Types */
    py_Type public_types[] = {
        tp_object,
        tp_type,
        tp_int,
        tp_float,
        tp_bool,
        tp_str,
        tp_list,
        tp_tuple,
        tp_slice,
        tp_range,
        tp_bytes,
        tp_dict,
        tp_property,
        tp_staticmethod,
        tp_classmethod,
        tp_super,
        tp_BaseException,
        tp_Exception,
    };

    for(int i = 0; i < c11__count_array(public_types); i++) {
        py_TypeInfo* ti = pk__type_info(public_types[i]);
        py_setdict(&self->builtins, ti->name, &ti->self);
    }

    py_newnotimplemented(py_emplacedict(&self->builtins, py_name("NotImplemented")));

    pk__add_module_linalg();
    pk__add_module_array2d();
    pk__add_module_colorcvt();

    // add modules
    pk__add_module_os();
    pk__add_module_sys();
    pk__add_module_io();
    pk__add_module_math();
    pk__add_module_dis();
    pk__add_module_random();
    pk__add_module_json();
    pk__add_module_gc();
    pk__add_module_time();
    pk__add_module_easing();
    pk__add_module_traceback();
    pk__add_module_enum();
    pk__add_module_inspect();
    pk__add_module_pickle();
    pk__add_module_importlib();

    pk__add_module_conio();
    pk__add_module_lz4();    // optional
    pk__add_module_libhv();  // optional
    pk__add_module_pkpy();

    // add python builtins
    do {
        bool ok;
        ok = py_exec(kPythonLibs_builtins, "<builtins>", EXEC_MODE, &self->builtins);
        if(!ok) goto __ABORT;
        break;
    __ABORT:
        py_printexc();
        c11__abort("failed to load python builtins!");
    } while(0);

    self->main = *py_newmodule("__main__");
}

void VM__dtor(VM* self) {
    // destroy all objects
    ManagedHeap__dtor(&self->heap);
    // clear frames
    while(self->top_frame)
        VM__pop_frame(self);
    ModuleDict__dtor(&self->modules);
    TypeList__dtor(&self->types);
    FixedMemoryPool__dtor(&self->pool_frame);
    ValueStack__clear(&self->stack);
}

void VM__push_frame(VM* self, Frame* frame) {
    frame->f_back = self->top_frame;
    self->top_frame = frame;
}

void VM__pop_frame(VM* self) {
    assert(self->top_frame);
    Frame* frame = self->top_frame;
    // reset stack pointer

    self->stack.sp = frame->p0;
    // pop frame and delete
    self->top_frame = frame->f_back;
    Frame__delete(frame);
}

static void _clip_int(int* value, int min, int max) {
    if(*value < min) *value = min;
    if(*value > max) *value = max;
}

bool pk__parse_int_slice(py_Ref slice, int length, int* start, int* stop, int* step) {
    py_Ref s_start = py_getslot(slice, 0);
    py_Ref s_stop = py_getslot(slice, 1);
    py_Ref s_step = py_getslot(slice, 2);

    if(py_isnone(s_step))
        *step = 1;
    else {
        if(!py_checkint(s_step)) return false;
        *step = py_toint(s_step);
    }
    if(*step == 0) return ValueError("slice step cannot be zero");

    if(*step > 0) {
        if(py_isnone(s_start))
            *start = 0;
        else {
            if(!py_checkint(s_start)) return false;
            *start = py_toint(s_start);
            if(*start < 0) *start += length;
            _clip_int(start, 0, length);
        }
        if(py_isnone(s_stop))
            *stop = length;
        else {
            if(!py_checkint(s_stop)) return false;
            *stop = py_toint(s_stop);
            if(*stop < 0) *stop += length;
            _clip_int(stop, 0, length);
        }
    } else {
        if(py_isnone(s_start))
            *start = length - 1;
        else {
            if(!py_checkint(s_start)) return false;
            *start = py_toint(s_start);
            if(*start < 0) *start += length;
            _clip_int(start, -1, length - 1);
        }
        if(py_isnone(s_stop))
            *stop = -1;
        else {
            if(!py_checkint(s_stop)) return false;
            *stop = py_toint(s_stop);
            if(*stop < 0) *stop += length;
            _clip_int(stop, -1, length - 1);
        }
    }
    return true;
}

bool pk__normalize_index(int* index, int length) {
    if(*index < 0) *index += length;
    if(*index < 0 || *index >= length) { return IndexError("%d not in [0, %d)", *index, length); }
    return true;
}

py_Type pk_newtype(const char* name,
                   py_Type base,
                   const py_GlobalRef module,
                   void (*dtor)(void*),
                   bool is_python,
                   bool is_sealed) {
    py_Type index = pk_current_vm->types.length;
    py_TypeInfo* ti = TypeList__emplace(&pk_current_vm->types);
    py_TypeInfo* base_ti = base ? pk__type_info(base) : NULL;
    if(base_ti && base_ti->is_sealed) {
        c11__abort("type '%s' is not an acceptable base type", py_name2str(base_ti->name));
    }
    py_TypeInfo__ctor(ti, py_name(name), index, base, base_ti, module ? *module : *py_NIL());
    if(!dtor && base) dtor = base_ti->dtor;
    ti->dtor = dtor;
    ti->is_python = is_python;
    ti->is_sealed = is_sealed;
    return index;
}

py_Type py_newtype(const char* name, py_Type base, const py_GlobalRef module, void (*dtor)(void*)) {
    py_Type type = pk_newtype(name, base, module, dtor, false, false);
    if(module) py_setdict(module, py_name(name), py_tpobject(type));
    return type;
}

static bool
    prepare_py_call(py_TValue* buffer, py_Ref argv, py_Ref p1, int kwargc, const FuncDecl* decl) {
    const CodeObject* co = &decl->code;
    int decl_argc = decl->args.length;

    if(p1 - argv < decl_argc) {
        return TypeError("%s() takes %d positional arguments but %d were given",
                         co->name->data,
                         decl_argc,
                         (int)(p1 - argv));
    }

    py_TValue* t = argv;
    // prepare args
    memset(buffer, 0, co->nlocals * sizeof(py_TValue));
    c11__foreach(int, &decl->args, index) buffer[*index] = *t++;
    // prepare kwdefaults
    c11__foreach(FuncDeclKwArg, &decl->kwargs, kv) buffer[kv->index] = kv->value;

    // handle *args
    if(decl->starred_arg != -1) {
        int exceed_argc = p1 - t;
        py_Ref vargs = &buffer[decl->starred_arg];
        py_newtuple(vargs, exceed_argc);
        for(int j = 0; j < exceed_argc; j++) {
            py_tuple_setitem(vargs, j, t++);
        }
    } else {
        // kwdefaults override
        // def f(a, b, c=None)
        // f(1, 2, 3) -> c=3
        c11__foreach(FuncDeclKwArg, &decl->kwargs, kv) {
            if(t >= p1) break;
            buffer[kv->index] = *t++;
        }
        // not able to consume all args
        if(t < p1) return TypeError("too many arguments (%s)", co->name->data);
    }

    if(decl->starred_kwarg != -1) py_newdict(&buffer[decl->starred_kwarg]);

    for(int j = 0; j < kwargc; j++) {
        py_Name key = py_toint(&p1[2 * j]);
        int index = c11_smallmap_n2i__get(&decl->kw_to_index, key, -1);
        // if key is an explicit key, set as local variable
        if(index >= 0) {
            buffer[index] = p1[2 * j + 1];
        } else {
            // otherwise, set as **kwargs if possible
            if(decl->starred_kwarg == -1) {
                return TypeError("'%n' is an invalid keyword argument for %s()",
                                 key,
                                 co->name->data);
            } else {
                // add to **kwargs
                bool ok = py_dict_setitem_by_str(&buffer[decl->starred_kwarg],
                                                 py_name2str(key),
                                                 &p1[2 * j + 1]);
                if(!ok) return false;
            }
        }
    }
    return true;
}

FrameResult VM__vectorcall(VM* self, uint16_t argc, uint16_t kwargc, bool opcall) {
    pk_print_stack(self, self->top_frame, (Bytecode){0});

    py_Ref p1 = self->stack.sp - kwargc * 2;
    py_Ref p0 = p1 - argc - 2;
    // [callable, <self>, args..., kwargs...]
    //      ^p0                    ^p1      ^_sp

    // handle boundmethod, do a patch
    if(p0->type == tp_boundmethod) {
        assert(py_isnil(p0 + 1));  // self must be NULL
        py_TValue* slots = PyObject__slots(p0->_obj);
        p0[0] = slots[1];  // callable
        p0[1] = slots[0];  // self
        // [unbound, self, args..., kwargs...]
    }

    py_Ref argv = p0 + 1 + (int)py_isnil(p0 + 1);

    if(p0->type == tp_function) {
        // check stack overflow
        if(self->stack.sp > self->stack.end) {
            py_exception(tp_StackOverflowError, "");
            return RES_ERROR;
        }

        Function* fn = py_touserdata(p0);
        const CodeObject* co = &fn->decl->code;

        switch(fn->decl->type) {
            case FuncType_NORMAL: {
                bool ok = prepare_py_call(self->__vectorcall_buffer, argv, p1, kwargc, fn->decl);
                if(!ok) return RES_ERROR;
                // copy buffer back to stack
                self->stack.sp = argv + co->nlocals;
                memcpy(argv, self->__vectorcall_buffer, co->nlocals * sizeof(py_TValue));
                // submit the call
                if(!fn->cfunc) {
                    // python function
                    VM__push_frame(self, Frame__new(co, &fn->module, p0, argv, true));
                    return opcall ? RES_CALL : VM__run_top_frame(self);
                } else {
                    // decl-based binding
                    self->__curr_function = p0;
                    bool ok = py_callcfunc(fn->cfunc, co->nlocals, argv);
                    self->stack.sp = p0;
                    self->__curr_function = NULL;
                    return ok ? RES_RETURN : RES_ERROR;
                }
            }
            case FuncType_SIMPLE:
                if(p1 - argv != fn->decl->args.length) {
                    const char* fmt = "%s() takes %d positional arguments but %d were given";
                    TypeError(fmt, co->name->data, fn->decl->args.length, (int)(p1 - argv));
                    return RES_ERROR;
                }
                if(kwargc) {
                    TypeError("%s() takes no keyword arguments", co->name->data);
                    return RES_ERROR;
                }
                // [callable, <self>, args..., local_vars...]
                //      ^p0                    ^p1      ^_sp
                self->stack.sp = argv + co->nlocals;
                // initialize local variables to py_NIL
                memset(p1, 0, (char*)self->stack.sp - (char*)p1);
                // submit the call
                if(!fn->cfunc) {
                    // python function
                    VM__push_frame(self, Frame__new(co, &fn->module, p0, argv, true));
                    return opcall ? RES_CALL : VM__run_top_frame(self);
                } else {
                    // decl-based binding
                    self->__curr_function = p0;
                    bool ok = py_callcfunc(fn->cfunc, co->nlocals, argv);
                    self->stack.sp = p0;
                    self->__curr_function = NULL;
                    return ok ? RES_RETURN : RES_ERROR;
                }
            case FuncType_GENERATOR: {
                bool ok = prepare_py_call(self->__vectorcall_buffer, argv, p1, kwargc, fn->decl);
                if(!ok) return RES_ERROR;
                // copy buffer back to stack
                self->stack.sp = argv + co->nlocals;
                memcpy(argv, self->__vectorcall_buffer, co->nlocals * sizeof(py_TValue));
                Frame* frame = Frame__new(co, &fn->module, p0, argv, true);
                pk_newgenerator(py_retval(), frame, p0, self->stack.sp);
                self->stack.sp = p0;  // reset the stack
                return RES_RETURN;
            }
            default: c11__unreachable();
        };

        c11__unreachable();
        /*****************_py_call*****************/
    }

    if(p0->type == tp_nativefunc) {
        if(kwargc && p0->_cfunc != pk__object_new) {
            TypeError("nativefunc does not accept keyword arguments");
            return RES_ERROR;
        }
        bool ok = py_callcfunc(p0->_cfunc, p1 - argv, argv);
        self->stack.sp = p0;
        return ok ? RES_RETURN : RES_ERROR;
    }

    if(p0->type == tp_type) {
        // [cls, NULL, args..., kwargs...]
        py_Ref new_f = py_tpfindmagic(py_totype(p0), __new__);
        assert(new_f && py_isnil(p0 + 1));

        // prepare a copy of args and kwargs
        int span = self->stack.sp - argv;
        *self->stack.sp++ = *new_f;  // push __new__
        *self->stack.sp++ = *p0;     // push cls
        memcpy(self->stack.sp, argv, span * sizeof(py_TValue));
        self->stack.sp += span;
        // [new_f, cls, args..., kwargs...]
        if(VM__vectorcall(self, argc, kwargc, false) == RES_ERROR) return RES_ERROR;
        // by recursively using vectorcall, args and kwargs are consumed

        // try __init__
        // NOTE: previously we use `get_unbound_method` but here we just use `tpfindmagic`
        // >> [cls, NULL, args..., kwargs...]
        // >> py_retval() is the new instance
        py_Ref init_f = py_tpfindmagic(py_totype(p0), __init__);
        if(init_f) {
            // do an inplace patch
            *p0 = *init_f;              // __init__
            p0[1] = self->last_retval;  // self
            // [__init__, self, args..., kwargs...]
            if(VM__vectorcall(self, argc, kwargc, false) == RES_ERROR) return RES_ERROR;
            *py_retval() = p0[1];  // restore the new instance
        }
        // reset the stack
        self->stack.sp = p0;
        return RES_RETURN;
    }

    // handle `__call__` overload
    if(pk_loadmethod(p0, __call__)) {
        // [__call__, self, args..., kwargs...]
        return VM__vectorcall(self, argc, kwargc, opcall);
    }

    TypeError("'%t' object is not callable", p0->type);
    return RES_ERROR;
}

/****************************************/
void PyObject__dtor(PyObject* self) {
    py_TypeInfo* ti = pk__type_info(self->type);
    if(ti->dtor) ti->dtor(PyObject__userdata(self));
    if(self->slots == -1) NameDict__dtor(PyObject__dict(self));
}

static void mark_object(PyObject* obj);

void pk__mark_value(py_TValue* val) {
    if(val->is_ptr) mark_object(val->_obj);
}

void pk__mark_namedict(NameDict* dict) {
    for(int i = 0; i < dict->length; i++) {
        NameDict_KV* kv = c11__at(NameDict_KV, dict, i);
        pk__mark_value(&kv->value);
    }
}

void pk__tp_set_marker(py_Type type, void (*gc_mark)(void*)) {
    py_TypeInfo* ti = pk__type_info(type);
    assert(ti->gc_mark == NULL);
    ti->gc_mark = gc_mark;
}

static void mark_object(PyObject* obj) {
    if(obj->gc_marked) return;
    obj->gc_marked = true;

    if(obj->slots > 0) {
        py_TValue* p = PyObject__slots(obj);
        for(int i = 0; i < obj->slots; i++)
            pk__mark_value(p + i);
    } else if(obj->slots == -1) {
        NameDict* dict = PyObject__dict(obj);
        pk__mark_namedict(dict);
    }

    py_TypeInfo* ti = pk__type_info(obj->type);
    if(ti->gc_mark) ti->gc_mark(PyObject__userdata(obj));
}

void FuncDecl__gc_mark(const FuncDecl* self) {
    CodeObject__gc_mark(&self->code);
    for(int j = 0; j < self->kwargs.length; j++) {
        FuncDeclKwArg* kw = c11__at(FuncDeclKwArg, &self->kwargs, j);
        pk__mark_value(&kw->value);
    }
}

void CodeObject__gc_mark(const CodeObject* self) {
    for(int i = 0; i < self->consts.length; i++) {
        py_TValue* p = c11__at(py_TValue, &self->consts, i);
        pk__mark_value(p);
    }
    for(int i = 0; i < self->func_decls.length; i++) {
        FuncDecl_ decl = c11__getitem(FuncDecl_, &self->func_decls, i);
        FuncDecl__gc_mark(decl);
    }
}

void ManagedHeap__mark(ManagedHeap* self) {
    VM* vm = pk_current_vm;
    // mark value stack
    for(py_TValue* p = vm->stack.begin; p != vm->stack.end; p++) {
        pk__mark_value(p);
    }
    // mark ascii literals
    for(int i = 0; i < c11__count_array(vm->ascii_literals); i++) {
        pk__mark_value(&vm->ascii_literals[i]);
    }
    // mark modules
    ModuleDict__apply_mark(&vm->modules, mark_object);
    // mark types
    int types_length = vm->types.length;
    // 0-th type is placeholder
    for(py_Type i = 1; i < types_length; i++) {
        py_TypeInfo* ti = TypeList__get(&vm->types, i);
        // mark type object
        pk__mark_value(&ti->self);
        // mark common magic slots
        for(int j = 0; j < PK_MAGIC_SLOTS_COMMON_LENGTH; j++) {
            py_TValue* slot = ti->magic_0 + j;
            if(py_isnil(slot)) continue;
            pk__mark_value(slot);
        }
        // mark uncommon magic slots
        if(ti->magic_1) {
            for(int j = 0; j < PK_MAGIC_SLOTS_UNCOMMON_LENGTH; j++) {
                py_TValue* slot = ti->magic_1 + j;
                if(py_isnil(slot)) continue;
                pk__mark_value(slot);
            }
        }
        // mark type annotations
        pk__mark_value(&ti->annotations);
    }
    // mark frame
    for(Frame* frame = vm->top_frame; frame; frame = frame->f_back) {
        Frame__gc_mark(frame);
    }
    // mark vm's registers
    pk__mark_value(&vm->last_retval);
    pk__mark_value(&vm->curr_exception);
    for(int i = 0; i < c11__count_array(vm->reg); i++) {
        pk__mark_value(&vm->reg[i]);
    }
}

void pk_print_stack(VM* self, Frame* frame, Bytecode byte) {
    return;
    if(frame == NULL || py_isnil(&self->main)) return;

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
                py_Ref path = py_getdict(p, __path__);
                pk_sprintf(&buf, "<module '%v'>", py_tosv(path));
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

py_TypeInfo* pk__type_info(py_Type type) { return TypeList__get(&pk_current_vm->types, type); }

int py_replinput(char* buf, int max_size) {
    buf[0] = '\0';  // reset first char because we check '@' at the beginning

    int size = 0;
    bool multiline = false;
    printf(">>> ");

    while(true) {
        int c = pk_current_vm->callbacks.getchar();
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