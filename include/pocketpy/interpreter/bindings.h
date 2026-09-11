#pragma once

#include "pocketpy/pocketpy.h"

/* Exception-free `__next__` of the builtin iterators. `py_next()` calls these
 * directly so that exhausting a builtin iterator does not construct a
 * `StopIteration` object.
 *   1: a value was produced into `py_retval()`
 *   0: the iterator is exhausted; `py_retval()` holds the `StopIteration`
 *      value, or `nil` if there is none
 *  -1: an error occurred and an exception was set */
int generator__iternext(py_Ref self);
int array2d_like_iterator__iternext(py_Ref self);
int list_iterator__iternext(py_Ref self);
int tuple_iterator__iternext(py_Ref self);
int dict_items__iternext(py_Ref self);
int range_iterator__iternext(py_Ref self);
int str_iterator__iternext(py_Ref self);

/// Raise `StopIteration` for an `__iternext` result of `0`.
/// `py_retval()` must hold the value to carry, or `nil` for no value.
bool pk__raise_stopiteration() PY_RAISE;

/// Define the `__next__` magic method as the exception-based wrapper of
/// `name##__iternext`. Only the owning type binds it, so it stays file-local.
#define PK_DEFINE_NEXT_WRAPPER(name)                                                               \
    static bool name##__next__(int argc, py_Ref argv) {                                            \
        PY_CHECK_ARGC(1);                                                                          \
        int res = name##__iternext(argv);                                                          \
        if(res == -1) return false;                                                                \
        if(res == 0) return pk__raise_stopiteration();                                             \
        return true;                                                                               \
    }
