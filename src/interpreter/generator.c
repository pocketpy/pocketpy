#include "pocketpy/interpreter/generator.h"
#include "pocketpy/interpreter/frame.h"

void pk_newgenerator(py_Ref out, Frame* frame, int slots) {
    Generator* ud = py_newobject(out, tp_generator, slots, sizeof(Generator));
    ud->frame = frame;
    ud->state = 0;
}