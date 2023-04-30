#ifndef POCKETPY_C_H 
#define POCKETPY_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct pkpy_vm_handle* pkpy_vm;

//we mostly follow the lua api for these bindings
//the key difference being most methods return a bool, true if it succeeded
//false if it did not

//if a method returns false call this next method to check the error and clear it
//if this method returns false it means that no error was set, and no action is taken
//if it returns true it means there was an error and it was cleared, it will provide a string summary of the error in the message parameter (if it is not NULL)
//NOTE : you need to free the message that is passed back after you are done using it
//or else pass in null as message, and it will just print the message to stderr
bool pkpy_clear_error(pkpy_vm, const char** message);


pkpy_vm pkpy_vm_create(bool use_stdio, bool enable_os);
bool pkpy_vm_exec(pkpy_vm vm_handle, const char* source);
void pkpy_vm_destroy(pkpy_vm vm);


typedef int (*pkpy_function)(pkpy_vm); 

bool pkpy_push_function(pkpy_vm, pkpy_function);
bool pkpy_push_int(pkpy_vm, int);
bool pkpy_push_float(pkpy_vm, double);

bool pkpy_set_global(pkpy_vm, const char* name);
bool pkpy_get_global(pkpy_vm vm_handle, const char* name);

//first push callable you want to call
//then push the arguments to send
//argc is the number of arguments that was pushed (not counting the callable)
bool pkpy_call(pkpy_vm vm_handle, int argc);

//first push the object the method belongs to (self)
//then push the callable you want to call
//then push the the argments
//argc is the number of arguments that was pushed (not counting the callable or self)
bool pkpy_call_method(pkpy_vm vm_handle, int argc);


//we will break with the lua api here
//lua uses 1 as the index to the first pushed element for all of these functions
//but we will start counting at zero to match python
//we will allow negative numbers to count backwards from the top
bool pkpy_to_int(pkpy_vm vm_handle, int index, int* ret);


#ifdef __cplusplus
}
#endif

#endif
