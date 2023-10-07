#include "pocketpy/collections.h"

namespace pkpy
{
	void PyDeque::_register(VM *vm, PyObject *mod, PyObject *type)
	{
		vm->bind_default_constructor<PyDeque>(type);

		vm->bind(type, "__len__(self) -> int",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 return VAR(self.len);
				 });

		vm->bind(type, "printHelloWorld(self) -> None",
				 [](VM *vm, ArgsView args)
				 {
					 printf("TESTING HELLO WORLD!!\n");

					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 self.printHelloWorld();

					 printf("TESTED HELLO WORLD!!\n");
					 return vm->None;
				 });

		vm->bind(type, "appendleft(self, item) -> None",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 int item = _CAST(int, args[1]); // TODO: change to PyObject*
					 self.appendLeft(item);
					 return vm->None;
				 });

		vm->bind(type, "append(self, item) -> None",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 int item = _CAST(int, args[1]); //	TODO: change to PyObject*
					 self.append(item);
					 return vm->None;
				 });

		vm->bind(type, "popleft(self) -> PyObject",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 int tmp = self.popLeft(); // TODO: change to PyObject*
					 return py_var(vm, tmp);
				 });

		vm->bind(type,"pop(self) -> PyObject",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 int tmp = self.pop(); // TODO: change to PyObject*
					 return py_var(vm, tmp);
				 });


		vm->bind(type, "count(self, obj) -> int",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 int obj = _CAST(int, args[1]); // TODO: change to PyObject*
					 int cnt = self.count(obj);
					 return VAR(cnt);
				 });


		vm->bind(type, "print(self) -> None", // TODO: remove this later
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 self.print(vm);
					 return vm->None;
				 });

		vm->bind(type, "clear(self) -> None",
				 [](VM *vm, ArgsView args)
				 {
					 PyDeque &self = _CAST(PyDeque &, args[0]);
					 self.clear();
					 return vm->None;
				 });
	}

	void PyDeque::_gc_mark() const
	{
		// TODO: implement
	}
	void PyDeque::appendLeft(int item) // TODO: change to PyObject*
	{
		DequeNode *node = new DequeNode(item);
		this->dequeItems->push_front(node);
	}
	void PyDeque::append(int item) // TODO: change to PyObject*
	{
		// int val = _CAST(int, item);
		DequeNode *node = new DequeNode(item);
		this->dequeItems->push_back(node);
	}

	int PyDeque::popLeft() // TODO: change to PyObject*
	{
		if (this->dequeItems->empty())
		{
			throw std::runtime_error("pop from an empty deque");
		}
		DequeNode *node = this->dequeItems->pop_front();
		return node->obj;
	}

	int PyDeque::pop() // TODO: change to PyObject*
	{
		if (this->dequeItems->empty())
		{
			throw std::runtime_error("pop from an empty deque");
		}
		DequeNode *node = this->dequeItems->pop_back();
		return node->obj;
	}

	int PyDeque::count(int obj) // TODO: change to PyObject*
	{
		return this->dequeItems->count(obj);
	}
	void PyDeque::clear()
	{
		this->dequeItems->makeListEmpty();
	}
	void PyDeque::print(VM *vm)
	{
		// TODO: Helper function to print the deque, will be removed later
		this->dequeItems->printList();
	}

	void PyDeque::printHelloWorld()
	{
		printf("Hello World!\n");
		return;
	}

	void add_module_mycollections(VM *vm)
	{
		PyObject *mycollections = vm->new_module("collections");
		PyDeque::register_class(vm, mycollections);
	}
} // namespace pkpypkpy
