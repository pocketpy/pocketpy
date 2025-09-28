#include "pocketpy/objects/codeobject.h"
#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/name.h"
#include "pocketpy/interpreter/vm.h"

_Thread_local VM* pk_current_vm;

static bool pk_initialized;
static bool pk_finalized;

static VM pk_default_vm;
static VM* pk_all_vm[16];
static py_TValue _True, _False, _None, _NIL;

void py_initialize() {
    c11__rtassert(!pk_finalized);

    if(pk_initialized) {
        // c11__abort("py_initialize() can only be called once!");
        return;
    }

    pk_names_initialize();

    // check endianness
    int x = 1;
    bool is_little_endian = *(char*)&x == 1;
    if(!is_little_endian) c11__abort("is_little_endian != true");

    _Static_assert(sizeof(py_TValue) == 24, "sizeof(py_TValue) != 24");
    _Static_assert(offsetof(py_TValue, extra) == 4, "offsetof(py_TValue, extra) != 4");

    pk_current_vm = pk_all_vm[0] = &pk_default_vm;

    // initialize some convenient references
    py_newbool(&_True, true);
    py_newbool(&_False, false);
    py_newnone(&_None);
    py_newnil(&_NIL);
    VM__ctor(&pk_default_vm);

    pk_initialized = true;
}

void py_finalize() {
    if(pk_finalized) c11__abort("py_finalize() can only be called once!");
    pk_finalized = true;

    for(int i = 1; i < 16; i++) {
        VM* vm = pk_all_vm[i];
        if(vm) {
            // temp fix https://github.com/pocketpy/pocketpy/issues/315
            // TODO: refactor VM__ctor and VM__dtor
            pk_current_vm = vm;
            VM__dtor(vm);
            PK_FREE(vm);
        }
    }
    pk_current_vm = &pk_default_vm;
    VM__dtor(&pk_default_vm);
    pk_current_vm = NULL;

    pk_names_finalize();
}

int py_currentvm() {
    for(int i = 0; i < 16; i++) {
        if(pk_all_vm[i] == pk_current_vm) return i;
    }
    return -1;
}

void py_switchvm(int index) {
    if(index < 0 || index >= 16) c11__abort("invalid vm index");
    if(!pk_all_vm[index]) {
        pk_current_vm = pk_all_vm[index] = PK_MALLOC(sizeof(VM));
        memset(pk_current_vm, 0, sizeof(VM));
        VM__ctor(pk_all_vm[index]);
    } else {
        pk_current_vm = pk_all_vm[index];
    }
}

void py_resetvm() {
    VM* vm = pk_current_vm;
    VM__dtor(vm);
    memset(vm, 0, sizeof(VM));
    VM__ctor(vm);
}

void py_resetallvm() {
    for(int i = 0; i < 16; i++) {
        py_switchvm(i);
        py_resetvm();
    }
    py_switchvm(0);
}

void* py_getvmctx() { return pk_current_vm->ctx; }

void py_setvmctx(void* ctx) { pk_current_vm->ctx = ctx; }

py_Callbacks* py_callbacks() { return &pk_current_vm->callbacks; }

/////////////////////////////

void py_sys_setargv(int argc, char** argv) {
    py_GlobalRef sys = py_getmodule("sys");
    py_Ref argv_list = py_getdict(sys, py_name("argv"));
    py_list_clear(argv_list);
    for(int i = 0; i < argc; i++) {
        py_newstr(py_list_emplace(argv_list), argv[i]);
    }
}

void py_sys_settrace(py_TraceFunc func, bool reset) {
    TraceInfo* info = &pk_current_vm->trace_info;
    info->func = func;
    if(!reset) return;
    if(info->prev_loc.src) {
        PK_DECREF(info->prev_loc.src);
        info->prev_loc.src = NULL;
    }
    info->prev_loc.lineno = -1;
}

int py_gc_collect() {
    ManagedHeap* heap = &pk_current_vm->heap;
    return ManagedHeap__collect(heap);
}

/////////////////////////////

void* py_malloc(size_t size) { return PK_MALLOC(size); }

void* py_realloc(void* ptr, size_t size) { return PK_REALLOC(ptr, size); }

void py_free(void* ptr) { PK_FREE(ptr); }

/////////////////////////////

py_GlobalRef py_True() { return &_True; }

py_GlobalRef py_False() { return &_False; }

py_GlobalRef py_None() { return &_None; }

py_GlobalRef py_NIL() { return &_NIL; }

/////////////////////////////

const char* pk_opname(Opcode op) {
    const static char* OP_NAMES[] = {
#define OPCODE(name) #name,
#include "pocketpy/xmacros/opcodes.h"
#undef OPCODE
    };
    return OP_NAMES[op];
}
