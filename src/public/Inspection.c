#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vm.h"

py_StackRef py_inspect_currentfunction() {
    VM* vm = pk_current_vm;
    if(vm->curr_function >= vm->stack.sp) return NULL;
    return vm->curr_function;
}

py_GlobalRef py_inspect_currentmodule() {
    py_Frame* frame = pk_current_vm->top_frame;
    if(!frame) return NULL;
    return frame->module;
}

py_Frame* py_inspect_currentframe() { return pk_current_vm->top_frame; }

void py_newglobals(py_OutRef out) {
    py_Frame* frame = pk_current_vm->top_frame;
    py_Frame_newglobals(frame, out);
}

void py_newlocals(py_OutRef out) {
    py_Frame* frame = pk_current_vm->top_frame;
    py_Frame_newlocals(frame, out);
}
