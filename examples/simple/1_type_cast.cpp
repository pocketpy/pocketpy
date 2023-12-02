
/**
 * This example demonstrates the type casting feature of PocketPy.
 * It creates a virtual machine and cast PyObject* to different types.
*/

#include "pocketpy.h"

using namespace pkpy;


int main(){
    // Create a virtual machine
    VM* vm = new VM();

    PyObject* str_obj = py_var(vm, "hello world");
    // Cast PyObject* to Str type
    Str& str = py_cast<Str&>(vm, str_obj); 
    std::cout << "string: " << str.c_str() << std::endl; // hello world


    PyObject* int_obj = py_var(vm, 10);
    // Cast PyObject* to Int type
    int int_var = py_cast<int>(vm, int_obj);
    std::cout << "int: " << int_var << std::endl; // 10

    PyObject* float_obj = py_var(vm, 10.5);
    // Cast PyObject* to double type
    double float_var = py_cast<double>(vm, float_obj);
    std::cout << "float: " << float_var << std::endl; // 10.5

    // Dispose the virtual machine
    delete vm;
    return 0;
}
