#pragma once

#include <pocketpy.h>

namespace pybind11
{
    inline pkpy::VM* vm = nullptr;
    inline std::map<pkpy::PyVar, int*>* _ref_counts_map = nullptr;

    inline void initialize(bool enable_os = true)
    {
        vm = new pkpy::VM(enable_os);
        _ref_counts_map = new std::map<pkpy::PyVar, int*>();

        // use to keep alive PyObject, when the object is hold by C++ side.
        vm->heap._gc_marker_ex = [](pkpy::VM* vm)
        {
            for(auto iter = _ref_counts_map->begin(); iter != _ref_counts_map->end();)
            {
                auto ref_count = iter->second;
                if(*ref_count != 0)
                {
                    // if ref count is not zero, then mark it.
                    PK_OBJ_MARK(iter->first);
                    ++iter;
                }
                else
                {
                    // if ref count is zero, then delete it.
                    iter = _ref_counts_map->erase(iter);
                    delete ref_count;
                }
            }
        };
    }

    inline void finalize()
    {
        delete _ref_counts_map;
        delete vm;
    }

    enum class return_value_policy : uint8_t
    {
        /**
         *  This is the default return value policy, which falls back to the policy
         *  return_value_policy::take_ownership when the return value is a pointer.
         *  Otherwise, it uses return_value::move or return_value::copy for rvalue
         *  and lvalue references, respectively. See below for a description of what
         *  all of these different policies do.
         */
        automatic = 0,

        /**
         *  As above, but use policy return_value_policy::reference when the return
         *  value is a pointer. This is the default conversion policy for function
         *  arguments when calling Python functions manually from C++ code (i.e. via
         *  handle::operator()). You probably won't need to use this.
         */
        automatic_reference,

        /**
         *  Reference an existing object (i.e. do not create a new copy) and take
         *  ownership. Python will call the destructor and delete operator when the
         *  object's reference count reaches zero. Undefined behavior ensues when
         *  the C++ side does the same..
         */
        take_ownership,

        /**
         *  Create a new copy of the returned object, which will be owned by
         *  Python. This policy is comparably safe because the lifetimes of the two
         *  instances are decoupled.
         */
        copy,

        /**
         *  Use std::move to move the return value contents into a new instance
         *  that will be owned by Python. This policy is comparably safe because the
         *  lifetimes of the two instances (move source and destination) are
         *  decoupled.
         */
        move,

        /**
         *  Reference an existing object, but do not take ownership. The C++ side
         *  is responsible for managing the object's lifetime and deallocating it
         *  when it is no longer used. Warning: undefined behavior will ensue when
         *  the C++ side deletes an object that is still referenced and used by
         *  Python.
         */
        reference,

        /**
         *  This policy only applies to methods and properties. It references the
         *  object without taking ownership similar to the above
         *  return_value_policy::reference policy. In contrast to that policy, the
         *  function or property's implicit this argument (called the parent) is
         *  considered to be the the owner of the return value (the child).
         *  pybind11 then couples the lifetime of the parent to the child via a
         *  reference relationship that ensures that the parent cannot be garbage
         *  collected while Python is still using the child. More advanced
         *  variations of this scheme are also possible using combinations of
         *  return_value_policy::reference and the keep_alive call policy
         */
        reference_internal
    };
}  // namespace pybind11
