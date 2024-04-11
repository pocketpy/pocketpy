#include "pocketpy/collections.h"

namespace pkpy
{
    struct PyDequeIter // Iterator for the deque type
    {
        PY_CLASS(PyDequeIter, collections, _deque_iterator)
        PyObject *ref;
        bool is_reversed;
        std::deque<PyObject *>::iterator begin, end, current;
        std::deque<PyObject *>::reverse_iterator rbegin, rend, rcurrent;
        PyDequeIter(PyObject *ref, std::deque<PyObject *>::iterator begin, std::deque<PyObject *>::iterator end)
            : ref(ref), begin(begin), end(end), current(begin)
        {
            this->is_reversed = false;
        }
        PyDequeIter(PyObject *ref, std::deque<PyObject *>::reverse_iterator rbegin, std::deque<PyObject *>::reverse_iterator rend)
            : ref(ref), rbegin(rbegin), rend(rend), rcurrent(rbegin)
        {
            this->is_reversed = true;
        }
        void _gc_mark() const { PK_OBJ_MARK(ref); }
        static void _register(VM *vm, PyObject *mod, PyObject *type);
    };
    void PyDequeIter::_register(VM *vm, PyObject *mod, PyObject *type)
    {
        // Iterator for the deque type
        vm->_all_types[PK_OBJ_GET(Type, type)].subclass_enabled = false;
        vm->bind_notimplemented_constructor<PyDequeIter>(type);

        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject *obj)
                         { return obj; });
        vm->bind__next__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject *obj)
                         {
            PyDequeIter& self = _CAST(PyDequeIter&, obj);
            if(self.is_reversed){
                if(self.rcurrent == self.rend) return vm->StopIteration;
                PyObject* ret = *self.rcurrent;
                ++self.rcurrent;
                return ret;
            }
            else{
                if(self.current == self.end) return vm->StopIteration;
                PyObject* ret = *self.current;
                ++self.current;
                return ret;
            } });
    }
    struct PyDeque
    {
        PY_CLASS(PyDeque, collections, deque);
        PyDeque(VM *vm, PyObject *iterable, PyObject *maxlen); // constructor
        // PyDeque members
        std::deque<PyObject *> dequeItems;
        int maxlen = -1;                                                  // -1 means unbounded
        bool bounded = false;                                             // if true, maxlen is not -1
        void insertObj(bool front, bool back, int index, PyObject *item); // insert at index, used purely for internal purposes: append, appendleft, insert methods
        PyObject *popObj(bool front, bool back, PyObject *item, VM *vm);  // pop at index, used purely for internal purposes: pop, popleft, remove methods
        int findIndex(VM *vm, PyObject *obj, int start, int stop);        // find the index of the given object in the deque
        // Special methods
        static void _register(VM *vm, PyObject *mod, PyObject *type); // register the type
        void _gc_mark() const;                                        // needed for container types, mark all objects in the deque for gc
    };
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
        vm->bind__getitem__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1)
        {
            PyDeque &self = _CAST(PyDeque &, _0);
            i64 index = CAST(i64, _1);
            index = vm->normalized_index(index, self.dequeItems.size()); // error is handled by the vm->normalized_index
            return self.dequeItems[index];
        });
        // sets the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind__setitem__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1, PyObject* _2)
        {
            PyDeque &self = _CAST(PyDeque&, _0);
            i64 index = CAST(i64, _1);
            index = vm->normalized_index(index, self.dequeItems.size()); // error is handled by the vm->normalized_index
            self.dequeItems[index] = _2;
        });
        // erases the item at the given index, if index is negative, it will be treated as index + len(deque)
        // if the index is out of range, IndexError will be thrown --> required for [] operator
        vm->bind__delitem__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1)
        {
            PyDeque &self = _CAST(PyDeque&, _0);
            i64 index = CAST(i64, _1);
            index = vm->normalized_index(index, self.dequeItems.size()); // error is handled by the vm->normalized_index
            self.dequeItems.erase(self.dequeItems.begin() + index);
        });

        vm->bind__len__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0)
        {
            PyDeque &self = _CAST(PyDeque&, _0);
            return (i64)self.dequeItems.size();
        });

        vm->bind__iter__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0)
        {
            PyDeque &self = _CAST(PyDeque &, _0);
            return vm->heap.gcnew<PyDequeIter>(
                PyDequeIter::_type(vm), _0,
                self.dequeItems.begin(), self.dequeItems.end());
        });

        vm->bind__repr__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0)
        {
            if(vm->_repr_recursion_set.count(_0)) return VAR("[...]");
            const PyDeque &self = _CAST(PyDeque&, _0);
            SStream ss;
            ss << "deque([";
            vm->_repr_recursion_set.insert(_0);
            for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
            {
                ss << CAST(Str&, vm->py_repr(*it));
                if (it != self.dequeItems.end() - 1) ss << ", ";
            }
            vm->_repr_recursion_set.erase(_0);
            self.bounded ? ss << "], maxlen=" << self.maxlen << ")" : ss << "])";
            return VAR(ss.str());
        });

        // enables comparison between two deques, == and != are supported
        vm->bind__eq__(PK_OBJ_GET(Type, type), [](VM *vm, PyObject* _0, PyObject* _1)
        {
            const PyDeque &self = _CAST(PyDeque&, _0);
            if(!is_type(_0, PyDeque::_type(vm))) return vm->NotImplemented;
            const PyDeque &other = _CAST(PyDeque&, _1);
            if (self.dequeItems.size() != other.dequeItems.size()) return vm->False;
            for (int i = 0; i < self.dequeItems.size(); i++){
                if (vm->py_ne(self.dequeItems[i], other.dequeItems[i])) return vm->False;
            }
            return vm->True;
        });

        // clear the deque
        vm->bind(type, "clear(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     self.dequeItems.clear();
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
                         self.insertObj(false, true, -1, obj);
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
                     self.insertObj(false, true, -1, item);
                     return vm->None;
                 });
        // append at the beginning of the deque
        vm->bind(type, "appendleft(self, item) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *item = args[1];
                     self.insertObj(true, false, -1, item);
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
                     return self.popObj(false, true, nullptr, vm);
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
                     return self.popObj(true, false, nullptr, vm);
                 });
        // shallow copy of the deque
        vm->bind(type, "copy(self) -> deque",
                 [](VM *vm, ArgsView args)
                 {
                     auto _lock = vm->heap.gc_scope_lock(); // locking the heap
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *newDequeObj = vm->heap.gcnew<PyDeque>(PyDeque::_type(vm), vm, vm->None, vm->None); // create the empty deque
                     PyDeque &newDeque = _CAST(PyDeque &, newDequeObj);                                           // cast it to PyDeque so we can use its methods
                     for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
                         newDeque.insertObj(false, true, -1, *it);
                     return newDequeObj;
                 });
        // NEW: counts the number of occurrences of the given object in the deque
        vm->bind(type, "count(self, obj) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int cnt = 0, sz = self.dequeItems.size();
                     for (auto it = self.dequeItems.begin(); it != self.dequeItems.end(); ++it)
                     {
                         if (vm->py_eq((*it), obj))
                             cnt++;
                         if (sz != self.dequeItems.size())// mutating the deque during iteration is not allowed
                             vm->RuntimeError("deque mutated during iteration"); 
                     }
                     return VAR(cnt);
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
                         self.insertObj(true, false, -1, obj);
                         obj = vm->py_next(it);
                     }
                     return vm->None;
                 });
        // NEW: returns the index of the given object in the deque
        vm->bind(type, "index(self, obj, start=None, stop=None) -> int",
                 [](VM *vm, ArgsView args)
                 {
                     // Return the position of x in the deque (at or after index start and before index stop). Returns the first match or raises ValueError if not found.
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int start = CAST_DEFAULT(int, args[2], 0);
                     int stop = CAST_DEFAULT(int, args[3], self.dequeItems.size());
                     int index = self.findIndex(vm, obj, start, stop);
                     if (index < 0) vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in deque");
                     return VAR(index);
                 });
        // NEW: returns the index of the given object in the deque
        vm->bind(type, "__contains__(self, obj) -> bool",
                 [](VM *vm, ArgsView args)
                 {
                     // Return the position of x in the deque (at or after index start and before index stop). Returns the first match or raises ValueError if not found.
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     int start = 0, stop = self.dequeItems.size(); // default values
                     int index = self.findIndex(vm, obj, start, stop);
                     if (index != -1)
                         return VAR(true);
                     return VAR(false);
                 });
        // NEW: inserts the given object at the given index
        vm->bind(type, "insert(self, index, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int index = CAST(int, args[1]);
                     PyObject *obj = args[2];
                     if (self.bounded && self.dequeItems.size() == self.maxlen)
                         vm->IndexError("deque already at its maximum size");
                     else
                         self.insertObj(false, false, index, obj); // this index shouldn't be fixed using vm->normalized_index, pass as is
                     return vm->None;
                 });
        // NEW: removes the first occurrence of the given object from the deque
        vm->bind(type, "remove(self, obj) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     PyObject *obj = args[1];
                     PyObject *removed = self.popObj(false, false, obj, vm);
                     if (removed == nullptr)
                         vm->ValueError(_CAST(Str &, vm->py_repr(obj)) + " is not in list");
                     return vm->None;
                 });
        // NEW: reverses the deque
        vm->bind(type, "reverse(self) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     if (self.dequeItems.empty() || self.dequeItems.size() == 1)
                         return vm->None; // handle trivial cases
                     int sz = self.dequeItems.size();
                     for (int i = 0; i < sz / 2; i++)
                     {
                         PyObject *tmp = self.dequeItems[i];
                         self.dequeItems[i] = self.dequeItems[sz - i - 1]; // swapping
                         self.dequeItems[sz - i - 1] = tmp;
                     }
                     return vm->None;
                 });
        // NEW: rotates the deque by n steps
        vm->bind(type, "rotate(self, n=1) -> None",
                 [](VM *vm, ArgsView args)
                 {
                     PyDeque &self = _CAST(PyDeque &, args[0]);
                     int n = CAST(int, args[1]);

                     if (n != 0 && !self.dequeItems.empty()) // trivial case
                     {
                         PyObject *tmp; // holds the object to be rotated
                         int direction = n > 0 ? 1 : -1;
                         n = abs(n);
                         n = n % self.dequeItems.size(); // make sure n is in range
                         while (n--)
                         {
                             if (direction == 1)
                             {
                                 tmp = self.dequeItems.back();
                                 self.dequeItems.pop_back();
                                 self.dequeItems.push_front(tmp);
                             }
                             else
                             {
                                 tmp = self.dequeItems.front();
                                 self.dequeItems.pop_front();
                                 self.dequeItems.push_back(tmp);
                             }
                         }
                     }
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
                     for (PyObject *obj : self.dequeItems)
                     {
                         list.push_back(obj);
                     }
                     ret[0] = VAR(std::move(list));
                     if (self.bounded)
                         ret[1] = VAR(self.maxlen);
                     else
                         ret[1] = vm->None;
                     return VAR(ret);
                 });
    }
    /// @brief initializes a new PyDeque object, actual initialization is done in __init__
    PyDeque::PyDeque(VM *vm, PyObject *iterable, PyObject *maxlen)
    {

        if (!vm->py_eq(maxlen, vm->None)) // fix the maxlen first
        {
            int tmp = CAST(int, maxlen);
            if (tmp < 0)
                vm->ValueError("maxlen must be non-negative");
            else
            {
                this->maxlen = tmp;
                this->bounded = true;
            }
        }
        else
        {
            this->bounded = false;
            this->maxlen = -1;
        }
        if (!vm->py_eq(iterable, vm->None))
        {
            this->dequeItems.clear();              // clear the deque
            auto _lock = vm->heap.gc_scope_lock(); // locking the heap
            PyObject *it = vm->py_iter(iterable);  // strong ref
            PyObject *obj = vm->py_next(it);
            while (obj != vm->StopIteration)
            {
                this->insertObj(false, true, -1, obj);
                obj = vm->py_next(it);
            }
        }
    }
    int PyDeque::findIndex(VM *vm, PyObject *obj, int start, int stop)
    {
        // the following code is special purpose normalization for this method, taken from CPython: _collectionsmodule.c file
        if (start < 0)
        {
            start = this->dequeItems.size() + start; // try to fix for negative indices
            if (start < 0)
                start = 0;
        }
        if (stop < 0)
        {
            stop = this->dequeItems.size() + stop; // try to fix for negative indices
            if (stop < 0)
                stop = 0;
        }
        if (stop > this->dequeItems.size())
            stop = this->dequeItems.size();
        if (start > stop)
            start = stop;                                                                                                           // end of normalization
        PK_ASSERT(start >= 0 && start <= this->dequeItems.size() && stop >= 0 && stop <= this->dequeItems.size() && start <= stop); // sanity check
        int loopSize = std::min((int)(this->dequeItems.size()), stop);
        int sz = this->dequeItems.size();
        for (int i = start; i < loopSize; i++)
        {
            if (vm->py_eq(this->dequeItems[i], obj))
                return i;
            if (sz != this->dequeItems.size())// mutating the deque during iteration is not allowed
                vm->RuntimeError("deque mutated during iteration");
        }
        return -1;
    }

    /// @brief pops or removes an item from the deque
    /// @param front  if true, pop from the front of the deque
    /// @param back if true, pop from the back of the deque
    /// @param item if front and back is not set, remove the first occurrence of item from the deque
    /// @param vm is needed for the py_eq
    /// @return PyObject* if front or back is set, this is a pop operation and we return a PyObject*, if front and back are not set, this is a remove operation and we return the removed item or nullptr
    PyObject *PyDeque::popObj(bool front, bool back, PyObject *item, VM *vm)
    {
        // error handling
        if (front && back)
            throw std::runtime_error("both front and back are set"); // this should never happen
        if (front || back)
        {
            // front or back is set, we don't care about item, this is a pop operation and we return a PyObject*
            if (this->dequeItems.empty())
                throw std::runtime_error("pop from an empty deque"); // shouldn't happen
            PyObject *obj;
            if (front)
            {
                obj = this->dequeItems.front();
                this->dequeItems.pop_front();
            }
            else
            {
                obj = this->dequeItems.back();
                this->dequeItems.pop_back();
            }
            return obj;
        }
        else
        {
            // front and back are not set, we care about item, this is a remove operation and we return the removed item or nullptr
            int sz = this->dequeItems.size();
            for (auto it = this->dequeItems.begin(); it != this->dequeItems.end(); ++it)
            {
                bool found = vm->py_eq((*it), item);
                if (sz != this->dequeItems.size()) // mutating the deque during iteration is not allowed
                    vm->IndexError("deque mutated during iteration");
                if (found)
                {
                    PyObject *obj = *it; // keep a reference to the object for returning
                    this->dequeItems.erase(it);
                    return obj;
                }
            }
            return nullptr; // not found
        }
    }
    /// @brief inserts an item into the deque
    /// @param front if true, insert at the front of the deque
    /// @param back if true, insert at the back of the deque
    /// @param index if front and back are not set, insert at the given index
    /// @param item the item to insert
    /// @return true if the item was inserted successfully, false if the deque is bounded and is already at its maximum size
    void PyDeque::insertObj(bool front, bool back, int index, PyObject *item) // assume index is not fixed using the vm->normalized_index
    {
        // error handling
        if (front && back)
            throw std::runtime_error("both front and back are set"); // this should never happen
        if (front || back)
        {
            // front or back is set, we don't care about index
            if (this->bounded)
            {
                if (this->maxlen == 0)
                    return; // bounded and maxlen is 0, so we can't append
                else if (this->dequeItems.size() == this->maxlen)
                {
                    if (front)
                        this->dequeItems.pop_back(); // remove the last item
                    else if (back)
                        this->dequeItems.pop_front(); // remove the first item
                }
            }
            if (front)
                this->dequeItems.emplace_front(item);
            else if (back)
                this->dequeItems.emplace_back(item);
        }
        else
        {
            // front and back are not set, we care about index
            if (index < 0)
                index = this->dequeItems.size() + index; // try fixing for negative indices
            if (index < 0)                               // still negative means insert at the beginning
                this->dequeItems.push_front(item);
            else if (index >= this->dequeItems.size()) // still out of range means insert at the end
                this->dequeItems.push_back(item);
            else
                this->dequeItems.insert((this->dequeItems.begin() + index), item); // insert at the given index
        }
    }
    /// @brief marks the deque items for garbage collection
    void PyDeque::_gc_mark() const
    {
        for (PyObject *obj : this->dequeItems)
            PK_OBJ_MARK(obj);
    }
    /// @brief registers the PyDeque class
    /// @param vm is needed for the new_module and register_class
    void add_module_collections(VM *vm)
    {
        PyObject *mod = vm->new_module("collections");
        PyDeque::register_class(vm, mod);
        PyDequeIter::register_class(vm, mod);
        CodeObject_ code = vm->compile(kPythonLibs_collections, "collections.py", EXEC_MODE);
        vm->_exec(code, mod);
    }
} // namespace pkpypkpy
