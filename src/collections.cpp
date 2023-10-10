#include "pocketpy/collections.h"

namespace pkpy
{
    void PyDeque::_register(VM *vm, PyObject *mod, PyObject *type)
    {
        vm->bind_default_constructor<PyDeque>(type);
        
        vm->bind(type, "__init__(self) -> None",
                    [](VM *vm, ArgsView args)
                    {
                        PyDeque &self = _CAST(PyDeque &, args[0]);
                        PyObject *maxlen = args[1];
                        if (maxlen != vm->None)
                        {
                            // printf("TODO: implement deque.__init__(maxlen) -> None: %d\n", CAST(int, maxlen));
                        }

                        return vm->None;
                    });

        vm->bind(type, "__len__(self) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     return VAR(self.dequeItems->size());
                 });

        vm->bind(type, "append(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];
                     self.append(item);
                     return vm->None;
                 });

        vm->bind(type, "appendleft(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];
                     self.appendLeft(item);
                     return vm->None;
                 });

        vm->bind(type, "clear(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     self.clear();
                     return vm->None;
                 });

        vm->bind(type, "copy(self) -> deque",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyDeque *newDeque = new PyDeque();
                     DequeNode *p = self.dequeItems->head->next;
                     while (p != self.dequeItems->tail)
                     {
                         newDeque->append(p->obj);
                         p = p->next;
                     }
                     return vm->heap.gcnew<PyDeque>(PyDeque::_type(vm), *newDeque);
                 });

        vm->bind(type, "count(self, obj) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int cnt = self.count(vm, obj);
                     return VAR(cnt);
                 });

        vm->bind(type, "extend(self, iterable) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     PyObject *it = vm->py_iter(args[1]); // strong ref
                     PyObject *obj = vm->py_next(it);
                     while (obj != vm->StopIteration)
                     {
                         self.append(obj);
                         obj = vm->py_next(it);
                     }
                     return vm->None;
                 });

        vm->bind(type, "extendleft(self, iterable) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     PyObject *it = vm->py_iter(args[1]); // strong ref
                     PyObject *obj = vm->py_next(it);
                     while (obj != vm->StopIteration)
                     {
                         self.appendLeft(obj);
                         obj = vm->py_next(it);
                     }
                     return vm->None;
                 });

        vm->bind(type, "index(self, obj, start=-1, stop=-1) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     // Return the position of x in the deque (at or after index start and before index stop). Returns the first match or raises ValueError if not found.
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int start = CAST(int, args[2]);
                     int stop = CAST(int, args[3]);
                     int idx = self.findIndex(obj, start, stop);

                     if (idx == -1)
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in list");

                     return VAR(idx);
                 });

        vm->bind(type, "insert(self, index, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     // TODO: implement and validate

                     printf("TODO: implement deque.insert()");
                     return vm->None;
                 });

        vm->bind(type, "__repr__(self) -> str",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     std::stringstream ss = self.getRepr(vm);
                     return VAR(ss.str());
                 });

        vm->bind(type, "pop(self) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     return self.pop();
                 });

        vm->bind(type, "popleft(self) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     return self.popLeft();
                 });

        vm->bind(type, "remove(self, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     // TODO: implement and validate
                     printf("TODO: implement deque.index()");
                     return vm->None;
                 });

        vm->bind(type, "reverse(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     self.reverse();
                     return vm->None;
                 });
        vm->bind(type, "rotate(self, n=1) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     // TODO: implement and validate
                     printf("TODO: implement deque.rotate()");
                     return vm->None;
                 });

        // vm->bind(type,"maxlen",
        //          [](VM *vm, ArgsView args)
        //          {
        //             return VAR(-1);
        //          });
    }

    void PyDeque::_gc_mark() const
    {
        DequeNode *p = this->dequeItems->head->next;
        while (p != this->dequeItems->tail)
        {
            PK_OBJ_MARK(p->obj); // PK_OBJ_MARK is a macro that calls _gc_mark on the PK_OBJ
            p = p->next;
        }
    }
    std::stringstream PyDeque::getRepr(VM *vm)
    {
        std::stringstream ss;
        ss << "deque([";
        DequeNode *p = this->dequeItems->head->next;
        while (p != this->dequeItems->tail)
        {
            ss << CAST(Str &, vm->py_repr(p->obj));
            if (p->next != this->dequeItems->tail)
                ss << ", ";
            p = p->next;
        }
        ss << "])";
        return ss;
    }

    int PyDeque::findIndex(PyObject *obj, int startPos = -1, int endPos = -1)
    {

        if (startPos == -1)
            startPos = 0;
        if (endPos == -1)
            endPos = this->dequeItems->size();
        
        if (!(startPos <= endPos))
        {
            throw std::runtime_error("startPos<=endPos");
        }
        int cnt = 0;
        DequeNode *p = this->dequeItems->head->next;
        while (p != this->dequeItems->tail)
        {
            if (p->obj == obj)
            {
                if (startPos == -1 || cnt >= startPos)
                {
                    if (endPos == -1 || cnt < endPos)
                    {
                        return cnt;
                    }
                }
            }
            cnt++;
            p = p->next;
        }
        return -1;
    }
    void PyDeque::reverse()
    {
        this->dequeItems->reverse();
    }

    void PyDeque::appendLeft(PyObject *item)
    {
        DequeNode *node = new DequeNode(item);
        this->dequeItems->push_front(node);
    }
    void PyDeque::append(PyObject *item)
    {
        DequeNode *node = new DequeNode(item);
        this->dequeItems->push_back(node);
    }

    PyObject *PyDeque::popLeft()
    {
        if (this->dequeItems->empty())
        {
            throw std::runtime_error("pop from an empty deque");
        }
        DequeNode *node = this->dequeItems->pop_front();
        return node->obj;
    }

    PyObject *PyDeque::pop()
    {
        if (this->dequeItems->empty())
        {
            throw std::runtime_error("pop from an empty deque");
        }
        DequeNode *node = this->dequeItems->pop_back();
        return node->obj;
    }

    int PyDeque::count(VM *vm, PyObject *obj)
    {
        int cnt = 0;
        DequeNode *p = this->dequeItems->head->next;
        while (p != this->dequeItems->tail)
        {
            if (vm->py_equals(p->obj, obj))
                cnt++;
            p = p->next;
        }
        return cnt;
    }
    void PyDeque::clear()
    {
        this->dequeItems->makeListEmpty();
    }

    void add_module_mycollections(VM *vm)
    {
        PyObject *mycollections = vm->new_module("mycollections");
        PyDeque::register_class(vm, mycollections);
    }
} // namespace pkpypkpy
