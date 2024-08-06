#include "pocketpy/interpreter/generator.h"
#include "pocketpy/interpreter/frame.h"
#include "pocketpy/interpreter/vm.h"
#include "pocketpy/pocketpy.h"

void pk_newgenerator(py_Ref out, Frame* frame, int slots) {
    Generator* ud = py_newobject(out, tp_generator, slots, sizeof(Generator));
    ud->frame = frame;
    ud->state = 0;
}

static bool generator__next__(int argc, py_Ref argv){
    return true;
}

py_Type pk_generator__register() {
    py_Type type = pk_newtype("generator", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, generator__next__);
    return type;
}
