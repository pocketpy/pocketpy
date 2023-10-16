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

    vm->exec("try:");
    vm->exec("@indent print('{0}={1}}'.format(1, 2)')");
    vm->exec("    exit(1)");
    vm->exec("except ValueError:");
    vm->exec("    print('ValueError')");
    vm->exec("print('{{{}xxx{}x}}'.format(1, 2))");
    vm->exec("print('{{abc}}'.format())");

// assert "{{{}xxx{}x}}".format(1, 2) == "{1xxx2x}"
// assert "{{abc}}".format() == "{abc}"
    // cjson loads and dumps!
    // vm->exec("import cjson");
    // vm->exec("dict = {'a': 1, 'b': [1, 3, 'Hello World'], 'c': {'a': 4}, 'd': None, 'd': True }");
    // vm->exec("json_str = cjson.dumps(dict)");
    // vm->exec("print(json_str)");
    // vm->exec("loaded_dict = cjson.loads(json_str)");
    // vm->exec("print(loaded_dict)");

    delete vm;
    return 0;
}
