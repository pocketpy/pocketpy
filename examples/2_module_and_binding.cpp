
/**
 * This example demonstrate the process of creating a python module and
 * bind a function to it, as well as the procedure for calling the function.
*/

#include "pocketpy.h"

using namespace pkpy;


int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // Create a module
    PyObject* math_module = vm->new_module("math");

    // Bind a function named "add" to the module
    vm->bind(math_module, "add(a: int, b: int) -> int",
        [](VM* vm, ArgsView args){
            int a = py_cast<int>(vm, args[0]);
            int b = py_cast<int>(vm, args[1]);
            return py_var(vm, a + b);
        });
        
  
    // Call the "add" function
    PyObject* f_sum = math_module->attr("add");
    PyObject* result = vm->call(f_sum, py_var(vm, 4), py_var(vm, 5));
    std::cout << "Sum: " << py_cast<int>(vm, result) << std::endl;   // 9

    // Dispose the virtual machine
    delete vm;
    return 0;
}
