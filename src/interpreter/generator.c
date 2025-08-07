#include "pocketpy/interpreter/generator.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/pocketpy.h"
#include <stdbool.h>
#include <assert.h>

void pk_newgenerator(py_Ref out, py_Frame* frame, py_TValue* begin, py_TValue* end) {
    Generator* ud = py_newobject(out, tp_generator, 1, sizeof(Generator));
    ud->frame = frame;
    ud->state = 0;
    py_Ref tmp = py_getslot(out, 0);
    py_newlist(tmp);
    for(py_TValue* p = begin; p != end; p++) {
        py_list_append(tmp, p);
    }
}

void Generator__dtor(Generator* ud) {
    if(ud->frame) Frame__delete(ud->frame);
}

bool generator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Generator* ud = py_touserdata(argv);
    py_StackRef p0 = py_peek(0);
    VM* vm = pk_current_vm;
    if(ud->state == 2) return StopIteration();

    // reset frame->p0
    assert(!ud->frame->is_locals_special);
    int locals_offset = ud->frame->locals - ud->frame->p0;
    ud->frame->p0 = py_peek(0);
    ud->frame->locals = ud->frame->p0 + locals_offset;
    
    // restore the context
    py_Ref backup = py_getslot(argv, 0);
    int length = py_list_len(backup);
    py_TValue* p = py_list_data(backup);
    for(int i = 0; i < length; i++)
        py_push(&p[i]);
    py_list_clear(backup);

    // push frame
    VM__push_frame(vm, ud->frame);
    ud->frame = NULL;

    FrameResult res = VM__run_top_frame(vm);

    if(res == RES_ERROR) {
        ud->state = 2;  // end this generator immediately on error
        if(py_matchexc(tp_StopIteration)) {
            py_clearexc(p0);
            return true;
        }
        return false;
    }

    if(res == RES_YIELD) {
        // backup the context
        ud->frame = vm->top_frame;
        for(py_StackRef p = ud->frame->p0; p != vm->stack.sp; p++) {
            py_list_append(backup, p);
        }
        vm->stack.sp = ud->frame->p0;
        vm->top_frame = vm->top_frame->f_back;
        vm->recursion_depth--;
        ud->state = 1;
        return true;
    } else {
        assert(res == RES_RETURN);
        ud->state = 2;
        // raise StopIteration(<retval>)
        bool ok = py_tpcall(tp_StopIteration, 1, py_retval());
        if(!ok) return false;
        return py_raise(py_retval());
    }
}

py_Type pk_generator__register() {
    py_Type type = pk_newtype("generator", tp_object, NULL, (py_Dtor)Generator__dtor, false, true);
    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, generator__next__);
    return type;
}
