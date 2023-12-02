
/**
 * This example demonstrate the use of PocketPy as a scripting language.
 * It creates a virtual machine and execute a python script.
*/

#include "pocketpy.h"

using namespace pkpy;


int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // Print "hello world" to the console
    vm->exec("print('hello world')"); // hello world

    // List comprehension
    vm->exec("l = [i*i for i in range(1, 6)]");
    vm->exec("print(l)"); // [1, 4, 9, 16, 25]

    // Dispose the virtual machine
    delete vm;
    return 0;
}
