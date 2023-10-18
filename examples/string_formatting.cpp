#include <iostream>
#include "pocketpy.h"

using namespace pkpy;


int main(){
    // Create a virtual machine
    VM* vm = new VM();

    //prints: Hello, World!
    vm->exec("print(\"Hello, {}!\".format(\"World\"))");

    //prints: I love Python
    vm->exec("print('{} {} {}'.format('I', 'love', 'Python'))");

    //prints: I love Python
    vm->exec("print('{0} {1} {2}'.format('I', 'love', 'Python'))");
    
    //prints: Python love I
    vm->exec("print('{2} {1} {0}'.format('I', 'love', 'Python'))");

    //prints: pythonlovepython
    vm->exec("print('{0}{1}{0}'.format('python', 'love'))");

    //prints 
    vm->exec("print('{k}={v}'.format(k='key', v='value'))");

    vm->exec("print('{k}={k}'.format(k='key'))");
    
    vm->exec("print('{0}={1}'.format('{0}', '{1}'))");

    vm->exec("print('{{{0}}}'.format(1))");

    vm->exec("print('{0}{1}{1}'.format(1, 2, 3))");
    // vm->exec("\
    // try: \
    //     print('{0}={1}}'.format(1, 2)) \
    // except ValueError: \
    //     print('ValueError')"
    // );

    vm->exec("try:\n");
    vm->exec("    print('{0}={1}}'.format(1, 2))\n");
    vm->exec("    exit(1)\n");
    vm->exec("except ValueError:\n");
    vm->exec("    print('ValueError')\n");

    vm->exec("print('{{{}xxx{}x}}'.format(1, 2))");
    vm->exec("print('{{abc}}'.format())");

    delete vm;
    return 0;
}
