#include "pocketpy/collections.h"

namespace pkpy
{
    void PyDeque::_register(VM *vm, PyObject *mod, PyObject *type)
    {
        vm->bind(type, "__new__(cls, iterable=None, maxlen=None)",
                 [](VM *vm, ArgsView args)
                 {
                     Type cls_t = PK_OBJ_GET(Type, args[0]);
                     PyObject *iterable = args[1];
                     PyObject *maxlen = args[2];

                     return vm->heap.gcnew<PyDeque>(cls_t, vm, iterable, maxlen);
                 });

        // gets the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind(type, "__getitem__(self, index) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);

                     PyObject *item = self.getItem(index);
                     if (item == nullptr)
                     {
                         vm->IndexError("deque index out of range");
                         return vm->None;
                     }
                     return item;
                 });

        // sets the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind(type, "__setitem__(self, index, newValue) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);
                     PyObject *newValue = args[2];

                     bool success = self.setItem(index, newValue);
                     if (!success)
                     {
                         vm->IndexError("deque index out of range");
                         return vm->None;
                     }
                     return vm->None;
                 });

        // erases the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind(type, "__delitem__(self, index) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);

                     bool success = self.eraseItem(index);
                     if (!success)
                     {
                         vm->IndexError("deque index out of range");
                         return vm->None;
                     }
                     return vm->None;
                 });

        // returns the length of the deque
        vm->bind(type, "__len__(self) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     return VAR(self.dequeItems.size());
                 });

        // returns an iterator for the deque
        vm->bind(type, "__iter__(self) -> deque_iterator",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     return vm->heap.gcnew<PyDequeIter>(
                         PyDequeIter::_type(vm), args[0],
                         self.dequeItems.begin(), self.dequeItems.end());
                 });

        // returns a string representation of the deque
        vm->bind(type, "__repr__(self) -> str",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     std::stringstream ss = self.getRepr(vm);
                     return VAR(ss.str());
                 });

        // enables comparison between two deques, == and != are supported
        vm->bind(type, "__eq__(self, other) -> bool",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyDeque &other = _CAST(PyDeque &, args[1]);

                     if (self.dequeItems.size() != other.dequeItems.size())
                         return VAR(false);
                     for (int i = 0; i < self.dequeItems.size(); i++)
                     {
                         if (!vm->py_equals(self.dequeItems[i], other.dequeItems[i]))
                             return VAR(false);
                     }
                     return VAR(true);
                 });

        // clear the deque
        vm->bind(type, "clear(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     self.clear();
                     return vm->None;
                 });

        // extend the deque with the given iterable
        vm->bind(type, "extend(self, iterable) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock(); // locking the heap

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

        // append at the end of the deque
        vm->bind(type, "append(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];

                     self.append(item);
                     return vm->None;
                 });

        // append at the beginning of the deque
        vm->bind(type, "appendleft(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];

                     self.appendLeft(item);
                     return vm->None;
                 });

        // pop from the end of the deque
        vm->bind(type, "pop(self) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     if (self.dequeItems.empty())
                     {
                         vm->IndexError("pop from an empty deque");
                         return vm->None;
                     }
                     return self.pop();
                 });

        // pop from the beginning of the deque
        vm->bind(type, "popleft(self) -> PyObject",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     if (self.dequeItems.empty())
                     {
                         vm->IndexError("pop from an empty deque");
                         return vm->None;
                     }
                     return self.popLeft();
                 });

        // shallow copy of the deque
        vm->bind(type, "copy(self) -> deque",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock(); // locking the heap
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     // shallow copy
                     PyObject *newDequeObj = vm->heap.gcnew<PyDeque>(PyDeque::_type(vm), vm, vm->None, vm->None); // create the empty deque
                     PyDeque &newDeque = _CAST(PyDeque &, newDequeObj);                                           // cast it to PyDeque so we can use its methods
                     for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
                     {
                         newDeque.append(*it); // append each item to the new deque
                     }
                     return newDequeObj;
                 });

        //NEW: counts the number of occurences of the given object in the deque
        vm->bind(type, "count(self, obj) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];

                     return VAR(self.count(vm, obj));
                 });

        // NEW: extends the deque from the left
        vm->bind(type, "extendleft(self, iterable) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock();

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

        // NEW: returns the index of the given object in the deque
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
                     {
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in deque");
                         return vm->None;
                     }
                     return VAR(idx);
                 });

        // NEW: inserts the given object at the given index
        vm->bind(type, "insert(self, index, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);
                     PyObject *obj = args[2];

                     if (self.bounded && self.dequeItems.size() == self.maxlen)
                     {
                         vm->ValueError("deque already at its maximum size");
                         return vm->None;
                     }
                     self.insert(index, obj);
                     return vm->None;
                 });

        // NEW: removes the first occurence of the given object from the deque
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

        // NEW: reverses the deque
        vm->bind(type, "reverse(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);

                     self.reverse();
                     return vm->None;
                 });

        // NEW: rotates the deque by n steps
        vm->bind(type, "rotate(self, n=1) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int n = CAST(int, args[1]);
                     if (n!=0) // handle trivial case
                        self.rotate(n);
                     return vm->None;
                 });

        // NEW: getter and setter of property `maxlen`
        vm->bind_property(
            type, "maxlen: int",
            [](VM *vm, ArgsView args)
            {
                PyDeque &self = _CAST(PyDeque &, args[0]);
                if (self.bounded)
                    return VAR(self.maxlen);
                else
                    return vm->None;
            },
            [](VM *vm, ArgsView args)
            {
                vm->AttributeError("attribute 'maxlen' of 'collections.deque' objects is not writable");
                return vm->None;
            });

        // NEW: support pickle
        vm->bind(type, "__getnewargs__(self) -> tuple[list, int]",
            [](VM *vm, ArgsView args)
            {
                PyDeque &self = _CAST(PyDeque &, args[0]);
                Tuple ret(2);
                List list;
                for(PyObject* obj: self.dequeItems){
                    list.push_back(obj);
                }
                ret[0] = VAR(std::move(list));
                if(self.bounded) ret[1] = VAR(self.maxlen);
                else ret[1] = vm->None;
                return VAR(ret);
            });
    }

    /// @brief initializes a new PyDeque object
    /// @param vm required for the py_iter and max_len casting
    /// @param iterable a list-like object to initialize the deque with
    /// @param maxlen the maximum length of the deque, makes the deque bounded
    PyDeque::PyDeque(VM *vm, PyObject *iterable, PyObject *maxlen)
    {
        if (maxlen != vm->None)
        {
            this->maxlen = CAST(int, maxlen);
            if (this->maxlen < 0)
                vm->ValueError("maxlen must be non-negative");
            this->bounded = true;
        }
        else
        {
            this->bounded = false;
            this->maxlen = -1;
        }

        if (iterable != vm->None)
        {
            auto _lock = vm->heap.gc_scope_lock(); // locking the heap
            PyObject *it = vm->py_iter(iterable); // strong ref
            PyObject *obj = vm->py_next(it);
            while (obj != vm->StopIteration)
            {
                this->append(obj);
                obj = vm->py_next(it);
            }
        }
    }
    /// @brief returns the item at the given index, if index is negative, it will be treated as index + len(deque)
    /// @param index the index of the item to get
    /// @return PyObject* the item at the given index, nullptr if the index is out of range
    PyObject *PyDeque::getItem(int index)
    {
        index = this->fixIndex(index);
        if (index == -1) return nullptr;
        return this->dequeItems.at(index);
    }
    /// @brief sets the item at the given index, if index is negative, it will be treated as index + len(deque)
    /// @param index the index of the item to set
    /// @param item the newValue for the item at the given index
    /// @return true if the item was set successfully, false if the index is out of range
    bool PyDeque::setItem(int index, PyObject *item)
    {
        index = this->fixIndex(index);
        if (index == -1) return false;
        this->dequeItems.at(index) = item;
        return true;
    }
    /// @brief erases the item at the given index, if index is negative, it will be treated as index + len(deque)
    /// @param index the index of the item to erase
    /// @return true if the item was erased successfully, false if the index is out of range
    bool PyDeque::eraseItem(int index)
    {
        index = this->fixIndex(index);
        if (index == -1) return false;
        this->dequeItems.erase(this->dequeItems.begin() + index);
        return true;
    }
    /// @brief rotates the deque by n steps
    /// @param n the number of steps to rotate the deque by, can be -ve for left rotation, +ve for right rotation, can be out of range
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
    /// @brief removes the first occurence of the given item
    /// @param vm  is needed for the py_equals
    /// @param item the item to remove
    /// @return true if the item was removed successfully, false if the item was not found
    bool PyDeque::remove(VM *vm, PyObject *item) // removes the first occurence of the given item
    {
        for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
            if (vm->py_equals((*it), item))
            {
                this->dequeItems.erase(it);
                return true;
            }
        return false;
    }
    /// @brief inserts the given item at the given index, if index is negative, it will be treated as index + len(deque)
    /// @param index index at which the item will be inserted
    /// @param item the item to insert
    /// @return true if the item was inserted successfully, false if the index is out of range
    bool PyDeque::insert(int index, PyObject *item)
    {
        if (index < 0)
            index = this->dequeItems.size() + index; // adjust for the -ve indexing
        if (index < 0)
            this->dequeItems.push_front(item);
        else if (index >= this->dequeItems.size())
            this->dequeItems.push_back(item);
        else
            this->dequeItems.insert((this->dequeItems.begin() + index), item);
        return true;
    }
    /// @brief returns a string representation of the deque
    /// @param vm is needed for the py_repr and String casting
    /// @return std::stringstream the string representation of the deque
    std::stringstream PyDeque::getRepr(VM *vm)
    {
        std::stringstream ss;
        ss << "deque([";
        for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
        {
            ss << CAST(Str &, vm->py_repr(*it));
            if (it != this->dequeItems.end() - 1)
                ss << ", ";
        }
        if (this->bounded)
            ss << "], maxlen=" << this->maxlen << ")";
        else
            ss << "])";
        return ss;
    }
    /// @brief returns the index of the given object in the deque, can search in a range
    /// @param vm is needed for the py_equals
    /// @param obj the object to search for
    /// @param startPos start position of the search
    /// @param endPos end position of the search
    /// @return int the index of the given object in the deque, -1 if not found
    int PyDeque::findIndex(VM *vm, PyObject *obj, int startPos = -1, int endPos = -1)
    {
        if (startPos == -1)
            startPos = 0;
        if (endPos == -1)
            endPos = this->dequeItems.size();
        if (!(startPos <= endPos))
            return -1; // invalid range
        int loopSize = std::min((int)this->dequeItems.size(), endPos);
        for (int i = startPos; i < loopSize; i++)
            if (vm->py_equals(this->dequeItems[i], obj))
                return i;
        return -1;
    }
    /// @brief reverses the deque
    void PyDeque::reverse()
    {
        if (this->dequeItems.empty() || this->dequeItems.size() == 1)
            return; // handle trivial cases
        int sz = this->dequeItems.size();
        for (int i = 0; i < sz / 2; i++)
        {
            PyObject *tmp = this->dequeItems[i];
            this->dequeItems[i] = this->dequeItems[sz - i - 1]; // swapping
            this->dequeItems[sz - i - 1] = tmp;
        }
    }
    /// @brief appends the given item to the beginning of the deque
    /// @param item the item to append
    void PyDeque::appendLeft(PyObject *item)
    {
        if (this->bounded){ // handle bounded case
            if(this->maxlen == 0) return; // bounded and maxlen is 0, so we can't append
            else if (this->dequeItems.size() == this->maxlen)
                this->dequeItems.pop_back(); // remove the last item
        }
        this->dequeItems.emplace_front(item);
    }
    /// @brief appends the given item to the end of the deque
    /// @param item the item to append
    void PyDeque::append(PyObject *item)
    {
        if(this->bounded){ // handle bounded case
            if(this->maxlen == 0) return; // bounded and maxlen is 0, so we can't append
            else if (this->dequeItems.size() == this->maxlen)
                this->dequeItems.pop_front(); // remove the first item
        }
        this->dequeItems.emplace_back(item);
    }
    /// @brief pops the first item from the deque, i.e. beginning of the deque
    /// @return PyObject* the popped item
    PyObject *PyDeque::popLeft()
    {
        if (this->dequeItems.empty())
            throw std::runtime_error("pop from an empty deque");//shouldn't happen
        
        PyObject *obj = this->dequeItems.front();
        this->dequeItems.pop_front();
        return obj;
    }
    /// @brief pops the last item from the deque, i.e. end of the deque
    /// @return PyObject* the popped item
    PyObject *PyDeque::pop()
    {
        if (this->dequeItems.empty())
            throw std::runtime_error("pop from an empty deque"); //shouldn't happen
        
        PyObject *obj = this->dequeItems.back();
        this->dequeItems.pop_back();
        return obj;
    }
    /// @brief counts the number of occurences of the given object in the deque
    /// @param vm is needed for the py_equals
    /// @param obj the object to search for
    /// @return int the number of occurences of the given object in the deque
    int PyDeque::count(VM *vm, PyObject *obj)
    {
        int cnt = 0;
        for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
            if (vm->py_equals((*it), obj))
                cnt++;
        return cnt;
    }
    /// @brief clears the deque
    void PyDeque::clear()
    {
        this->dequeItems.clear();
    }
    /// @brief fixes the given index, if index is negative, it will be treated as index + len(deque)
    /// @param index the index to fix
    /// @return int the fixed index, -1 if the index is out of range
    int PyDeque::fixIndex(int index)
    {
        if (index < 0)
            index = this->dequeItems.size() + index;
        if (index < 0 || index >= this->dequeItems.size())
            return -1;
        return index;
    }
    /// @brief marks the deque items for garbage collection
    void PyDeque::_gc_mark() const
    {
        for (PyObject *obj : this->dequeItems)
        {
            PK_OBJ_MARK(obj);
        }
    }
    /// @brief registers the PyDeque class
    /// @param vm is needed for the new_module and register_class
    void add_module_collections(VM *vm)
    {
        PyObject* mod = vm->new_module("collections");
        PyDeque::register_class(vm, mod);
        CodeObject_ code = vm->compile(kPythonLibs["collections"], "collections.py", EXEC_MODE);
        vm->_exec(code, mod);
    }
} // namespace pkpypkpy
