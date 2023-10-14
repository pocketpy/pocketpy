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
                     return VAR(self.dequeItems.size());
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
                     // TODO: STILL MEMORY LEAKING??
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyDeque *newDeque = new PyDeque();
                     for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
                     {
                         newDeque->append(*it);
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
                     int idx = self.findIndex(vm, obj, start, stop);

                     if (idx == -1)
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in list");

                     return VAR(idx);
                 });

        vm->bind(type, "insert(self, index, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);
                     PyObject *obj = args[2];

                     // TODO: HANDLE MAX SIZE CASE LATER -> Throw IndexError

                     self.insert(index, obj);
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
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     bool removed = self.remove(vm, obj);

                     if (!removed)
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in list");

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
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int n = CAST(int, args[1]);
                     self.rotate(n);
                     return vm->None;
                 });

        // vm->bind(type,"maxlen",
        //          [](VM *vm, ArgsView args)
        //          {
        //             return VAR(-1);
        //          });
    }

    void PyDeque::rotate(int n)
    {
        int direction = n > 0 ? 1 : -1;
        int sz = this->dequeItems.size();

        n = abs(n);
        n = n % sz; // make sure n is in range

        for (int i = 0; i < n; i++)
        {
            if (direction == 1)
            {
                PyObject *tmp = this->dequeItems.back();
                this->dequeItems.pop_back();
                this->dequeItems.push_front(tmp);
            }
            else
            {
                PyObject *tmp = this->dequeItems.front();
                this->dequeItems.pop_front();
                this->dequeItems.push_back(tmp);
            }
        }
    }

    bool PyDeque::remove(VM *vm, PyObject *item)
    {
        for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
        {
            if (vm->py_equals((*it), item))
            {
                this->dequeItems.erase(it);
                return true;
            }
        }
        return false;
    }

    bool PyDeque::insert(int index, PyObject *item)
    {
        if (index < 0)
            this->dequeItems.push_front(item);
        else if (index >= this->dequeItems.size())
            this->dequeItems.push_back(item);
        else
            this->dequeItems.insert((this->dequeItems.begin() + index), item);

        return true;
    }

    void PyDeque::_gc_mark() const
    {
        // TODO: HOW TO IMPLEMENT THIS?
    }

    std::stringstream PyDeque::getRepr(VM *vm)
    {
        std::stringstream ss;
        int sz = this->dequeItems.size();
        ss << "deque([";
        for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
        {
            ss << CAST(Str &, vm->py_repr(*it));
            if (it != this->dequeItems.end() - 1)
            {
                ss << ", ";
            }
        }
        ss << "])";
        return ss;
    }

    int PyDeque::findIndex(VM *vm, PyObject *obj, int startPos = -1, int endPos = -1)
    {

        if (startPos == -1)
            startPos = 0;
        if (endPos == -1)
            endPos = this->dequeItems.size();

        if (!(startPos <= endPos))
        {
            throw std::runtime_error("startPos<=endPos");
        }
        int dequeSize = this->dequeItems.size();

        for (int i = startPos; i < dequeSize && i < endPos; i++)
        {
            if (vm->py_equals(this->dequeItems[i], obj))
            {
                return i;
            }
        }
        return -1;
    }

    void PyDeque::reverse()
    {
        int sz = this->dequeItems.size();
        for (int i = 0; i < sz / 2; i++)
        {
            PyObject *tmp = this->dequeItems[i];
            this->dequeItems[i] = this->dequeItems[sz - i - 1];
            this->dequeItems[sz - i - 1] = tmp;
        }
    }

    void PyDeque::appendLeft(PyObject *item)
    {
        this->dequeItems.emplace_front(item);
    }
    void PyDeque::append(PyObject *item)
    {
        this->dequeItems.emplace_back(item);
    }

    PyObject *PyDeque::popLeft()
    {
        if (this->dequeItems.empty())
        {
            throw std::runtime_error("pop from an empty deque");
        }
        PyObject *obj = this->dequeItems.front();
        this->dequeItems.pop_front();
        return obj;
    }

    PyObject *PyDeque::pop()
    {
        if (this->dequeItems.empty())
        {
            throw std::runtime_error("pop from an empty deque");
        }
        PyObject *obj = this->dequeItems.back();
        this->dequeItems.pop_back();
        return obj;
    }

    int PyDeque::count(VM *vm, PyObject *obj)
    {
        int cnt = 0;

        for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
        {
            if (vm->py_equals((*it), obj))
            {
                cnt++;
            }
        }
        return cnt;
    }

    void PyDeque::clear()
    {
        this->dequeItems.clear();
    }

    void add_module_mycollections(VM *vm)
    {
        PyObject *mycollections = vm->new_module("mycollections");
        PyDeque::register_class(vm, mycollections);
    }
} // namespace pkpypkpy
