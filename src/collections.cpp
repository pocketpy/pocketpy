#include "pocketpy/collections.h"

namespace pkpy
{
	void PyDeque::_register(VM *vm, PyObject *mod, PyObject *type)
	{
		vm->bind_default_constructor<PyDeque>(type);

		vm->bind(type, "__len__(self) -> int",
				 [](VM *vm, ArgsView args){
					PyDeque& self = _CAST(PyDeque&, args[0]);
					return VAR(self.len); 
			});

		vm->bind(type, "printHelloWorld(self) -> None",
				 [](VM *vm, ArgsView args){
					PyDeque& self = _CAST(PyDeque&, args[0]);
					self.printHelloWorld();
					return vm->None;
			});
	}

	void PyDeque::printHelloWorld()
	{
		printf("Hello World!\n");
	}
	
	void add_module_mycollections(VM *vm)
	{
		PyObject *mycollections = vm->new_module("collections");
		PyDeque::register_class(vm, mycollections);
	}
} // namespace pkpypkpy
