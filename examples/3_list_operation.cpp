
/**
 * This example demonstrates how to build a List object and how to access its elements.
 * It creates a python module named "math_utils" and bind a function named "average" to it.
 * It exercises creating a List and accessing its elements.
 * It also demonstrates how to cast a python object to a C++ type.
*/

#include "pocketpy.h"

using namespace pkpy;


int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // Create a module
    PyObject* math_utils = vm->new_module("math_utils");

    // Bind a function named "average" to the module
    vm->bind(math_utils, "average(l: List[float]) -> float",
        [](VM* vm, ArgsView args){
            // Cast the argument to a list
            List& l = py_cast<List&>(vm, args[0]);
            double sum = 0;

            // Calculate the sum of the list by iterating through the list
            for(auto& e : l){
                sum += py_cast<double>(vm, e);
            }
            return py_var(vm, sum / l.size());
        });

    // Create a list of numbers and covert it to a python object
    List numbers;
    numbers.push_back(py_var(vm, 1.0));
    numbers.push_back(py_var(vm, 2.0));
    numbers.push_back(py_var(vm, 3.0));
    PyObject* list = py_var(vm, std::move(numbers));

    // Call the "average" function
    PyObject* f_average = math_utils->attr("average");
    PyObject* result = vm->call(f_average, list);
    std::cout << "Average: " << py_cast<double>(vm, result) << std::endl;   // 2

    // Dispose the virtual machine
    delete vm;
    return 0;
}
