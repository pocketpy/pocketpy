#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"

void py_printexc(){
    pk_VM* vm = pk_current_vm;
    if(vm->has_error){
        assert(vm->last_retval.type == tp_exception);
    }else{
        vm->_stdout("NoneType: None\n");
    }
}


void py_formatexc(char *out){

}