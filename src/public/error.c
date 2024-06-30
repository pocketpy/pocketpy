#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"


py_Error* py_lasterror(){
    return pk_current_vm->last_error;
}

void py_Error__print(py_Error* self){
    abort();
}

