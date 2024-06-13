#include "pocketpy/common/any.h"

#include <string.h>

void c11_userdata__ctor(c11_userdata* self, void* ptr, int size){
    memcpy(self, ptr, size);
}
