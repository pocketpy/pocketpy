#include "pocketpy/interpreter/vm.h"
#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/interpreter/generator.h"
#include "pocketpy/interpreter/modules.h"
#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/interpreter/types.h"
#include "pocketpy/common/_generated.h"
#include "pocketpy/objects/exception.h"
#include "pocketpy/pocketpy.h"
#include <stdbool.h>
#include <assert.h>

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

static void pk_default_flush() { fflush(stdout); }

static int pk_default_getchr() { return getchar(); }

void py_profiler_begin() {
    LineProfiler* lp = &pk_current_vm->line_profiler;
    TraceInfo* trace_info = &pk_current_vm->trace_info;
    if(trace_info->func == NULL) py_sys_settrace(LineProfiler_tracefunc, true);
    c11__rtassert(trace_info->func == LineProfiler_tracefunc);
    LineProfiler__begin(lp);
}

void py_profiler_end() {
    LineProfiler* lp = &pk_current_vm->line_profiler;
    LineProfiler__end(lp);
}

void py_profiler_reset() {
    LineProfiler* lp = &pk_current_vm->line_profiler;
    LineProfiler__reset(lp);
}

char* py_profiler_report() {
    LineProfiler* lp = &pk_current_vm->line_profiler;
    if(lp->enabled) LineProfiler__end(lp);
    c11_string* s = LineProfiler__get_report(lp);
    char* s_dup = c11_strdup(s->data);
    c11_string__delete(s);
    return s_dup;
}

void LineProfiler_tracefunc(py_Frame* frame, enum py_TraceEvent event) {
    LineProfiler* lp = &pk_current_vm->line_profiler;
    if(lp->enabled) LineProfiler__tracefunc_internal(lp, frame, event);
}

static int BinTree__cmp_cstr(void* lhs, void* rhs) {
    const char* l = (const char*)lhs;
    const char* r = (const char*)rhs;
    return strcmp(l, r);
}

void VM__ctor(VM* self) {
    self->top_frame = NULL;

    const static BinTreeConfig modules_config = {
        .f_cmp = BinTree__cmp_cstr,
        .need_free_key = false,
    };
    BinTree__ctor(&self->modules, "", py_NIL(), &modules_config);
    c11_vector__ctor(&self->types, sizeof(TypePointer));

    self->builtins = NULL;
    self->main = NULL;

    self->callbacks.importfile = pk_default_importfile;
    self->callbacks.lazyimport = NULL;
    self->callbacks.print = pk_default_print;
    self->callbacks.flush = pk_default_flush;
    self->callbacks.getchr = pk_default_getchr;

    self->last_retval = *py_NIL();
    self->unhandled_exc = *py_NIL();

    self->recursion_depth = 0;
    self->max_recursion_depth = 1000;

    self->ctx = NULL;
    self->curr_class = NULL;
    self->curr_decl_based_function = NULL;
    memset(&self->trace_info, 0, sizeof(TraceInfo));
    memset(&self->watchdog_info, 0, sizeof(WatchdogInfo));
    LineProfiler__ctor(&self->line_profiler);

    FixedMemoryPool__ctor(&self->pool_frame, sizeof(py_Frame), 32);

    ManagedHeap__ctor(&self->heap);
    self->stack.sp = self->stack.begin;
    self->stack.end = self->stack.begin + PK_VM_STACK_SIZE;

    CachedNames__ctor(&self->cached_names);
    NameDict__ctor(&self->compile_time_funcs, PK_TYPE_ATTR_LOAD_FACTOR);

    /* Init Builtin Types */
    // 0: unused
    TypePointer* placeholder = c11_vector__emplace(&self->types);
    placeholder->ti = NULL;
    placeholder->dtor = NULL;

#define validate(t, expr)                                                                          \
    if(t != (expr)) abort()

    validate(tp_object, pk_newtype("object", tp_nil, NULL, NULL, true, false));
    validate(tp_type, pk_newtype("type", tp_object, NULL, NULL, false, true));
    pk_object__register();

    validate(tp_int, pk_newtype("int", tp_object, NULL, NULL, false, true));
    validate(tp_float, pk_newtype("float", tp_object, NULL, NULL, false, true));
    validate(tp_bool, pk_newtype("bool", tp_object, NULL, NULL, false, true));
    pk_number__register();

    validate(tp_str, pk_str__register());
    validate(tp_str_iterator, pk_str_iterator__register());

    validate(tp_list, pk_list__register());
    validate(tp_tuple, pk_tuple__register());
    validate(tp_list_iterator, pk_list_iterator__register());
    validate(tp_tuple_iterator, pk_tuple_iterator__register());

    validate(tp_slice, pk_slice__register());
    validate(tp_range, pk_range__register());
    validate(tp_range_iterator, pk_range_iterator__register());
    validate(tp_module, pk_module__register());

    validate(tp_function, pk_function__register());
    validate(tp_nativefunc, pk_nativefunc__register());
    validate(tp_boundmethod, pk_boundmethod__register());

    validate(tp_super, pk_super__register());
    validate(tp_BaseException, pk_BaseException__register());
    validate(tp_Exception, pk_Exception__register());
    validate(tp_bytes, pk_bytes__register());
    validate(tp_namedict, pk_namedict__register());
    validate(tp_locals, pk_newtype("locals", tp_object, NULL, NULL, false, true));
    validate(tp_code, pk_code__register());

    validate(tp_dict, pk_dict__register());
    validate(tp_dict_iterator, pk_dict_items__register());

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

    // inject some builtin exceptions
#define INJECT_BUILTIN_EXC(name, TBase)                                                            \
    do {                                                                                           \
        py_Type type = pk_newtype(#name, TBase, self->builtins, NULL, false, true);                \
        py_setdict(self->builtins, py_name(#name), py_tpobject(type));                             \
        validate(tp_##name, type);                                                                 \
    } while(0)

    INJECT_BUILTIN_EXC(SystemExit, tp_BaseException);
    INJECT_BUILTIN_EXC(KeyboardInterrupt, tp_BaseException);

    validate(tp_StopIteration, pk_StopIteration__register());
    py_setdict(self->builtins, py_name("StopIteration"), py_tpobject(tp_StopIteration));

    INJECT_BUILTIN_EXC(SyntaxError, tp_Exception);
    INJECT_BUILTIN_EXC(RecursionError, tp_Exception);
    INJECT_BUILTIN_EXC(OSError, tp_Exception);
    INJECT_BUILTIN_EXC(NotImplementedError, tp_Exception);
    INJECT_BUILTIN_EXC(TypeError, tp_Exception);
    INJECT_BUILTIN_EXC(IndexError, tp_Exception);
    INJECT_BUILTIN_EXC(ValueError, tp_Exception);
    INJECT_BUILTIN_EXC(RuntimeError, tp_Exception);
    INJECT_BUILTIN_EXC(TimeoutError, tp_Exception);
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
        py_TypeInfo* ti = pk_typeinfo(public_types[i]);
        py_setdict(self->builtins, ti->name, &ti->self);
    }

    py_newnotimplemented(py_emplacedict(self->builtins, py_name("NotImplemented")));

    pk__add_module_vmath();
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
    pk__add_module_base64();
    pk__add_module_importlib();
    pk__add_module_unicodedata();

    pk__add_module_conio();
    pk__add_module_lz4();       // optional
    pk__add_module_libhv();     // optional
    pk__add_module_cute_png();  // optional
    pk__add_module_pkpy();

    // add python builtins
    do {
        bool ok;
        ok = py_exec(kPythonLibs_builtins, "<builtins>", EXEC_MODE, self->builtins);
        if(!ok) goto __ABORT;
        break;
    __ABORT:
        py_printexc();
        c11__abort("failed to load python builtins!");
    } while(0);

    self->main = py_newmodule("__main__");
}

void VM__dtor(VM* self) {
    // reset traceinfo
    py_sys_settrace(NULL, true);
    LineProfiler__dtor(&self->line_profiler);
    // destroy all objects
    ManagedHeap__dtor(&self->heap);
    // clear frames
    while(self->top_frame) {
        VM__pop_frame(self);
    }
    BinTree__dtor(&self->modules);
    FixedMemoryPool__dtor(&self->pool_frame);
    CachedNames__dtor(&self->cached_names);
    NameDict__dtor(&self->compile_time_funcs);
    c11_vector__dtor(&self->types);
}

void VM__push_frame(VM* self, py_Frame* frame) {
    frame->f_back = self->top_frame;
    self->top_frame = frame;
    self->recursion_depth++;
    if(self->trace_info.func) self->trace_info.func(frame, TRACE_EVENT_PUSH);
}

void VM__pop_frame(VM* self) {
    assert(self->top_frame);
    py_Frame* frame = self->top_frame;
    if(self->trace_info.func) self->trace_info.func(frame, TRACE_EVENT_POP);
    // reset stack pointer
    self->stack.sp = frame->p0;
    // pop frame and delete
    self->top_frame = frame->f_back;
    Frame__delete(frame);
    self->recursion_depth--;
}

static void _clip_int(int* value, int min, int max) {
    if(*value < min) *value = min;
    if(*value > max) *value = max;
}

bool pk__parse_int_slice(py_Ref slice,
                         int length,
                         int* restrict start,
                         int* restrict stop,
                         int* restrict step) {
    if(py_isint(slice)) {
        int index = py_toint(slice);
        bool ok = pk__normalize_index(&index, length);
        if(!ok) return false;
        *start = index;
        *stop = index + 1;
        *step = 1;
        return true;
    }

    if(!py_istype(slice, tp_slice)) c11__abort("pk__parse_int_slice(): not a slice object");

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
    if(*index < 0 || *index >= length) return IndexError("%d not in [0, %d)", *index, length);
    return true;
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
        py_Ref data = py_newtuple(vargs, exceed_argc);
        for(int j = 0; j < exceed_argc; j++) {
            data[j] = *t++;
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
        py_Name key = (py_Name)py_toint(&p1[2 * j]);
        int index = c11_smallmap_n2d__get(&decl->kw_to_index, key, -1);
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
                bool ok =
                    py_dict_setitem(&buffer[decl->starred_kwarg], py_name2ref(key), &p1[2 * j + 1]);
                if(!ok) return false;
            }
        }
    }
    return true;
}

FrameResult VM__vectorcall(VM* self, uint16_t argc, uint16_t kwargc, bool opcall) {
#ifndef NDEBUG
    pk_print_stack(self, self->top_frame, (Bytecode){0});

    if(py_checkexc()) {
        const char* name = py_tpname(self->unhandled_exc.type);
        c11__abort("unhandled exception `%s` was set!", name);
    }
#endif

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
        Function* fn = py_touserdata(p0);
        const CodeObject* co = &fn->decl->code;

        switch(fn->decl->type) {
            case FuncType_NORMAL: {
                bool ok = prepare_py_call(self->vectorcall_buffer, argv, p1, kwargc, fn->decl);
                if(!ok) return RES_ERROR;
                // copy buffer back to stack
                self->stack.sp = argv + co->nlocals;
                memcpy(argv, self->vectorcall_buffer, co->nlocals * sizeof(py_TValue));
                // submit the call
                if(!fn->cfunc) {
                    // python function
                    VM__push_frame(self, Frame__new(co, p0, fn->module, fn->globals, argv, false));
                    return opcall ? RES_CALL : VM__run_top_frame(self);
                } else {
                    // decl-based binding
                    self->curr_decl_based_function = p0;
                    bool ok = py_callcfunc(fn->cfunc, co->nlocals, argv);
                    self->stack.sp = p0;
                    self->curr_decl_based_function = NULL;
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
                    VM__push_frame(self, Frame__new(co, p0, fn->module, fn->globals, argv, false));
                    return opcall ? RES_CALL : VM__run_top_frame(self);
                } else {
                    // decl-based binding
                    self->curr_decl_based_function = p0;
                    bool ok = py_callcfunc(fn->cfunc, co->nlocals, argv);
                    self->stack.sp = p0;
                    self->curr_decl_based_function = NULL;
                    return ok ? RES_RETURN : RES_ERROR;
                }
            case FuncType_GENERATOR: {
                bool ok = prepare_py_call(self->vectorcall_buffer, argv, p1, kwargc, fn->decl);
                if(!ok) return RES_ERROR;
                // copy buffer back to stack
                self->stack.sp = argv + co->nlocals;
                memcpy(argv, self->vectorcall_buffer, co->nlocals * sizeof(py_TValue));
                py_Frame* frame = Frame__new(co, p0, fn->module, fn->globals, argv, false);
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
void FuncDecl__gc_mark(const FuncDecl* self, c11_vector* p_stack) {
    CodeObject__gc_mark(&self->code, p_stack);
    for(int j = 0; j < self->kwargs.length; j++) {
        FuncDeclKwArg* kw = c11__at(FuncDeclKwArg, &self->kwargs, j);
        pk__mark_value(&kw->value);
    }
}

void CodeObject__gc_mark(const CodeObject* self, c11_vector* p_stack) {
    for(int i = 0; i < self->consts.length; i++) {
        py_TValue* p = c11__at(py_TValue, &self->consts, i);
        pk__mark_value(p);
    }
    for(int i = 0; i < self->func_decls.length; i++) {
        FuncDecl_ decl = c11__getitem(FuncDecl_, &self->func_decls, i);
        FuncDecl__gc_mark(decl, p_stack);
    }
}

static void pk__mark_value_func(py_Ref val, void* ctx) {
    c11_vector* p_stack = ctx;
    pk__mark_value(val);
}

void ManagedHeap__mark(ManagedHeap* self) {
    VM* vm = pk_current_vm;
    c11_vector* p_stack = &self->gc_roots;
    assert(p_stack->length == 0);

    // mark value stack
    for(py_TValue* p = vm->stack.begin; p < vm->stack.sp; p++) {
        // assert(p->type != tp_nil);
        pk__mark_value(p);
    }
    // mark modules
    BinTree__apply_mark(&vm->modules, p_stack);
    // mark cached names
    for(int i = 0; i < vm->cached_names.entries.length; i++) {
        CachedNames_KV* kv = c11_chunkedvector__at(&vm->cached_names.entries, i);
        pk__mark_value(&kv->val);
    }
    // mark compile time functions
    for(int i = 0; i < vm->compile_time_funcs.capacity; i++) {
        NameDict_KV* kv = &vm->compile_time_funcs.items[i];
        if(kv->key == NULL) continue;
        pk__mark_value(&kv->value);
    }
    // mark types
    int types_length = vm->types.length;
    // 0-th type is placeholder
    for(py_Type i = 1; i < types_length; i++) {
        py_TypeInfo* ti = c11__getitem(TypePointer, &vm->types, i).ti;
        pk__mark_value(&ti->self);
        pk__mark_value(&ti->annotations);
    }
    // mark frame
    for(py_Frame* frame = vm->top_frame; frame; frame = frame->f_back) {
        Frame__gc_mark(frame, p_stack);
    }
    // mark vm's registers
    pk__mark_value(&vm->last_retval);
    pk__mark_value(&vm->unhandled_exc);
    for(int i = 0; i < c11__count_array(vm->reg); i++) {
        pk__mark_value(&vm->reg[i]);
    }
    // mark user func
    if(vm->callbacks.gc_mark) vm->callbacks.gc_mark(pk__mark_value_func, p_stack);
    /*****************************/
    while(p_stack->length > 0) {
        PyObject* obj = c11_vector__back(PyObject*, p_stack);
        c11_vector__pop(p_stack);

        assert(obj->gc_marked);

        if(obj->slots > 0) {
            py_TValue* p = PyObject__slots(obj);
            for(int i = 0; i < obj->slots; i++)
                pk__mark_value(p + i);
        } else if(obj->slots == -1) {
            NameDict* dict = PyObject__dict(obj);
            for(int i = 0; i < dict->capacity; i++) {
                NameDict_KV* kv = &dict->items[i];
                if(kv->key == NULL) continue;
                pk__mark_value(&kv->value);
            }
        }

        void* ud = PyObject__userdata(obj);
        switch(obj->type) {
            case tp_list: {
                List* self = ud;
                for(int i = 0; i < self->length; i++) {
                    py_TValue* val = c11__at(py_TValue, self, i);
                    pk__mark_value(val);
                }
                break;
            }
            case tp_dict: {
                Dict* self = ud;
                for(int i = 0; i < self->entries.length; i++) {
                    DictEntry* entry = c11__at(DictEntry, &self->entries, i);
                    if(py_isnil(&entry->key)) continue;
                    pk__mark_value(&entry->key);
                    pk__mark_value(&entry->val);
                }
                break;
            }
            case tp_generator: {
                Generator* self = ud;
                if(self->frame) Frame__gc_mark(self->frame, p_stack);
                break;
            }
            case tp_function: {
                function__gc_mark(ud, p_stack);
                break;
            }
            case tp_BaseException: {
                BaseException* self = ud;
                pk__mark_value(&self->args);
                pk__mark_value(&self->inner_exc);
                c11__foreach(BaseExceptionFrame, &self->stacktrace, frame) {
                    pk__mark_value(&frame->locals);
                    pk__mark_value(&frame->globals);
                }
                break;
            }
            case tp_code: {
                CodeObject* self = ud;
                CodeObject__gc_mark(self, p_stack);
                break;
            }
            case tp_chunked_array2d: {
                c11_chunked_array2d__mark(ud, p_stack);
                break;
            }
        }
    }
}
