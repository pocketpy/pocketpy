#pragma once

#include "pocketpy/common/memorypool.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/heap.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/modules.h"
#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/interpreter/name.h"
#include "pocketpy/interpreter/types.h"

// TODO:
// 1. __eq__ and __ne__ fallbacks
// 2. un-cleared exception detection
// 3. super()
// 4. stack balance guanrantee
// 5. stack effect of each opcode
// 6. py_TypeInfo

typedef struct TraceInfo {
    SourceLocation prev_loc;
    py_TraceFunc func;
} TraceInfo;

typedef struct WatchdogInfo {
    py_i64 timeout;
    py_i64 last_reset_time;
} WatchdogInfo;

typedef struct VM {
    py_Frame* top_frame;

    ModuleDict modules;
    TypeList types;

    py_TValue builtins;  // builtins module
    py_TValue main;      // __main__ module

    py_Callbacks callbacks;

    py_TValue last_retval;
    py_TValue curr_exception;

    int recursion_depth;
    int max_recursion_depth;

    bool is_curr_exc_handled;  // handled by try-except block but not cleared yet

    py_TValue reg[8];  // users' registers
    void* ctx;         // user-defined context

    py_StackRef curr_class;
    py_StackRef curr_decl_based_function;
    TraceInfo trace_info;
    WatchdogInfo watchdog_info;
    py_TValue vectorcall_buffer[PK_MAX_CO_VARNAMES];

    InternedNames names;
    FixedMemoryPool pool_frame;
    ManagedHeap heap;
    ValueStack stack;  // put `stack` at the end for better cache locality
} VM;

void VM__ctor(VM* self);
void VM__dtor(VM* self);

void VM__push_frame(VM* self, py_Frame* frame);
void VM__pop_frame(VM* self);

bool pk__parse_int_slice(py_Ref slice,
                         int length,
                         int* restrict start,
                         int* restrict stop,
                         int* restrict step);
bool pk__normalize_index(int* index, int length);

#define pk__mark_value(val)                                                                        \
    if((val)->is_ptr && !(val)->_obj->gc_marked) {                                                 \
        PyObject* obj = (val)->_obj;                                                               \
        obj->gc_marked = true;                                                                     \
        c11_vector__push(PyObject*, p_stack, obj);                                                 \
    }

bool pk__object_new(int argc, py_Ref argv);
py_TypeInfo* pk__type_info(py_Type type);

bool pk_wrapper__self(int argc, py_Ref argv);

const char* pk_op2str(py_Name op);

typedef enum FrameResult {
    RES_ERROR = 0,
    RES_RETURN = 1,
    RES_CALL = 2,
    RES_YIELD = 3,
} FrameResult;

FrameResult VM__run_top_frame(VM* self);

py_Type pk_newtype(const char* name,
                   py_Type base,
                   const py_GlobalRef module,
                   void (*dtor)(void*),
                   bool is_python,
                   bool is_sealed);

FrameResult VM__vectorcall(VM* self, uint16_t argc, uint16_t kwargc, bool opcall);

const char* pk_opname(Opcode op);

int pk_arrayview(py_Ref self, py_TValue** p);
bool pk_wrapper__arrayequal(py_Type type, int argc, py_Ref argv);
bool pk_arrayiter(py_Ref val);
bool pk_arraycontains(py_Ref self, py_Ref val);

bool pk_loadmethod(py_StackRef self, py_Name name);
bool pk_callmagic(py_Name name, int argc, py_Ref argv);

bool pk_exec(CodeObject* co, py_Ref module);
bool pk_execdyn(CodeObject* co, py_Ref module, py_Ref globals, py_Ref locals);

/// Assumes [a, b] are on the stack, performs a binary op.
/// The result is stored in `self->last_retval`.
/// The stack remains unchanged.
bool pk_stack_binaryop(VM* self, py_Name op, py_Name rop);

void pk_print_stack(VM* self, py_Frame* frame, Bytecode byte);

// type registration
void pk_object__register();
void pk_number__register();
py_Type pk_str__register();
py_Type pk_str_iterator__register();
py_Type pk_bytes__register();
py_Type pk_dict__register();
py_Type pk_dict_items__register();
py_Type pk_list__register();
py_Type pk_tuple__register();
py_Type pk_array_iterator__register();
py_Type pk_slice__register();
py_Type pk_function__register();
py_Type pk_nativefunc__register();
py_Type pk_boundmethod__register();
py_Type pk_range__register();
py_Type pk_range_iterator__register();
py_Type pk_BaseException__register();
py_Type pk_Exception__register();
py_Type pk_StopIteration__register();
py_Type pk_super__register();
py_Type pk_property__register();
py_Type pk_staticmethod__register();
py_Type pk_classmethod__register();
py_Type pk_generator__register();
py_Type pk_namedict__register();
py_Type pk_code__register();

py_TValue pk_builtins__register();

/* mappingproxy */
void pk_mappingproxy__namedict(py_Ref out, py_Ref object);