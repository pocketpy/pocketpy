#include "pocketpy.h"

using namespace pkpy;

int main(){
    // Create a virtual machine
    VM* vm = new VM();

    // cjson loads and dumps!
    vm->exec("import cjson");
    vm->exec("dict = {'a': 1, 'b': [1, 3, 'Hello World'], 'c': {'a': 4}, 'd': None, 'd': True }");
    vm->exec("json_str = cjson.dumps(dict)");
    vm->exec("print(json_str)");
    vm->exec("loaded_dict = cjson.loads(json_str)");
    vm->exec("print(loaded_dict)");

    delete vm;
    return 0;
}
