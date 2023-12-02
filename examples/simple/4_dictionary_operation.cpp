
/**
 * This example illustrate use of Dict in PocketPy.
 * It creates a python module named "employee" and bind four functions to it.
 * It exercises setting and getting elements in a Dict.
*/


#include "pocketpy.h"

using namespace pkpy;


int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // Create "employee" module
    PyObject* employee_module = vm->new_module("employee");

    // Bind a function named "get_first_name" to the module
    vm->bind(employee_module, "get_first_name(employee: Dict) -> str",
        "Returns first_name of the employee",  // docstring
        [](VM* vm, ArgsView args){
            // Cast the argument to a dictionary
            Dict& employee = CAST(Dict&, args[0]);
            // Access the first_name field and return it
            return employee.try_get(VAR("first_name"));
        });

    // Bind a function named "get_last_name" to the module
    vm->bind(employee_module, "get_last_name(employee: Dict) -> str",
        "Returns last_name of the employee",  // docstring
        [](VM* vm, ArgsView args){
            // Cast the argument to a dictionary
            Dict& employee = CAST(Dict&, args[0]);
            // Access the last_name field and return it
            return employee.try_get(VAR("last_name"));
        });

    // Bind a function named "get_salary" to the module
    // It accepts a dictionary as argument and returns a float
    vm->bind(employee_module, "get_salary(employee: Dict) -> float",
        "Returns salary of the employee",  // docstring
        [](VM* vm, ArgsView args){
            // Cast the argument to a dictionary
            Dict& employee = CAST(Dict&, args[0]);
            // Access the salary field and return it
            return employee.try_get(VAR("salary"));
        });

    // Bind a function named "love_coding" to the module
    // It accepts a dictionary as argument and returns a bool
    vm->bind(employee_module, "love_coding(employee: Dict) -> bool",
        "Returns Yes if the employee loves coding, No otherwise",  // docstring
        [](VM* vm, ArgsView args){
            // Cast the argument to a dictionary
            Dict& employee = CAST(Dict&, args[0]);

            // Access the hobbies field and cast it to a list
            List& hobbies = CAST(List&, employee.try_get(VAR("hobbies")));

            // Iterate through the list and check if the employee loves coding
            for(auto& e : hobbies){
                if(CAST(Str&, e) == Str("Coding")){
                    return VAR("Yes");
                }
            }
            return VAR("No");
        });

    // Create employee dictionary covert it to a python object
    Dict employee(vm);
    employee.set(VAR("first_name"), VAR("John"));
    employee.set(VAR("last_name"), VAR("Doe"));
    employee.set(VAR("age"), VAR(30));
    employee.set(VAR("salary"), VAR(10000.0));
    List hobbies;
    hobbies.push_back(VAR("Reading"));
    hobbies.push_back(VAR("Walking"));
    hobbies.push_back(VAR("Coding"));
    employee.set(VAR("hobbies"), VAR(std::move(hobbies)));
    PyObject* employee_obj = VAR(std::move(employee));

    // Call the "get_first_name" function
    PyObject* f_get_first_name = employee_module->attr("get_first_name");
    PyObject* first_name = vm->call(f_get_first_name, employee_obj);
    std::cout << "First name: " << CAST(Str&, first_name) << std::endl;   // First name: John

    // Call the "get_last_name" function
    PyObject* f_get_last_name = employee_module->attr("get_last_name");
    PyObject* last_name = vm->call(f_get_last_name, employee_obj);
    std::cout << "Last name: " << CAST(Str&, last_name) << std::endl;   // Last name: Doe
 
    // Call the "get_salary" function
    PyObject* f_get_salary = employee_module->attr("get_salary");
    PyObject* salary = vm->call(f_get_salary, employee_obj);
    std::cout << "Salary: "<< CAST(double, salary) << std::endl;   // Salary: 10000

    // Call the "love_coding" function
    PyObject* f_love_coding = employee_module->attr("love_coding");
    PyObject* love_coding = vm->call(f_love_coding, employee_obj);
    std::cout << "Loves coding: " << CAST(Str&, love_coding) << std::endl;   // Loves coding: Yes

    // Dispose the virtual machine
    delete vm;
    return 0;
}
