#include "pocketpy/objects/object.h"

typedef struct pkpy_VM{
    PyVar True;
    PyVar False;
    PyVar None;
    PyVar NotImplemented;
    PyVar Ellipsis;
} pkpy_VM;

void pkpy_VM__ctor(pkpy_VM* self);
void pkpy_VM__dtor(pkpy_VM* self);
