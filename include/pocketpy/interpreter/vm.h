#pragma once

#include "pocketpy/common/memorypool.h"
#include "pocketpy/common/name.h"
#include "pocketpy/objects/codeobject.h"
#include "pocketpy/objects/bintree.h"
#include "pocketpy/objects/container.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/heap.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/typeinfo.h"
#include "pocketpy/interpreter/line_profiler.h"
#include <time.h>

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
    clock_t max_reset_time;
} WatchdogInfo;

typedef struct TypePointer {
    py_TypeInfo* ti;
    py_Dtor dtor;
} TypePointer;

typedef struct py_ModuleInfo {
    c11_string* name;
    c11_string* package;
    c11_string* path;
    py_GlobalRef self;  // weakref to the original module object
} py_ModuleInfo;

typedef struct VM {
    py_Frame* top_frame;

    BinTree modules;
    c11_vector /*TypePointer*/ types;

    py_GlobalRef builtins;  // builtins module
    py_GlobalRef main;      // __main__ module

    py_Callbacks callbacks;

    py_TValue last_retval;
    py_TValue unhandled_exc;

    int recursion_depth;
    int max_recursion_depth;

    py_TValue reg[8];  // users' registers
    void* ctx;         // user-defined context

    CachedNames cached_names;
    NameDict compile_time_funcs;

    py_StackRef curr_class;
    py_StackRef curr_decl_based_function;   // this is for get current function without frame
    TraceInfo trace_info;
    WatchdogInfo watchdog_info;
    LineProfiler line_profiler;
    py_TValue vectorcall_buffer[PK_MAX_CO_VARNAMES];

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

bool pk__object_new(int argc, py_Ref argv);

bool pk_wrapper__self(int argc, py_Ref argv);

const char* pk_op2str(py_Name op);

typedef enum FrameResult {
    RES_ERROR = 0,
    RES_RETURN = 1,
    RES_CALL = 2,
    RES_YIELD = 3,
} FrameResult;

FrameResult VM__run_top_frame(VM* self);

FrameResult VM__vectorcall(VM* self, uint16_t argc, uint16_t kwargc, bool opcall);

const char* pk_opname(Opcode op);

int pk_arrayview(py_Ref self, py_TValue** p);
bool pk_wrapper__arrayequal(py_Type type, int argc, py_Ref argv);
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

bool pk_format_object(VM* self, py_Ref val, c11_sv spec);

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
py_Type pk_list_iterator__register();
py_Type pk_tuple_iterator__register();
py_Type pk_slice__register();
py_Type pk_function__register();
py_Type pk_nativefunc__register();
py_Type pk_boundmethod__register();
py_Type pk_range__register();
py_Type pk_range_iterator__register();
py_Type pk_module__register();
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

py_GlobalRef pk_builtins__register();

/* mappingproxy */
void pk_mappingproxy__namedict(py_Ref out, py_Ref object);