#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

typedef struct Range {
    py_i64 start;
    py_i64 stop;
    py_i64 step;
} Range;

static bool range__new__(int argc, py_Ref argv) {
    Range* ud = py_newobject(py_retval(), tp_range, 0, sizeof(Range));
    switch(argc - 1) {  // skip cls
        case 1: {
            PY_CHECK_ARG_TYPE(1, tp_int);
            ud->start = 0;
            ud->stop = py_toint(py_arg(1));
            ud->step = 1;
            break;
        }
        case 2:
            PY_CHECK_ARG_TYPE(1, tp_int);
            PY_CHECK_ARG_TYPE(2, tp_int);
            ud->start = py_toint(py_arg(1));
            ud->stop = py_toint(py_arg(2));
            ud->step = 1;
            break;
        case 3:
            PY_CHECK_ARG_TYPE(1, tp_int);
            PY_CHECK_ARG_TYPE(2, tp_int);
            PY_CHECK_ARG_TYPE(3, tp_int);
            ud->start = py_toint(py_arg(1));
            ud->stop = py_toint(py_arg(2));
            ud->step = py_toint(py_arg(3));
            break;
        default: return TypeError("range() expected at most 3 arguments, got %d", argc - 1);
    }
    if(ud->step == 0) return ValueError("range() step must not be zero");
    return true;
}

static bool range__iter__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return py_tpcall(tp_range_iterator, 1, argv);
}

py_Type pk_range__register() {
    py_Type type = pk_newtype("range", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, range__new__);
    py_bindmagic(type, __iter__, range__iter__);
    return type;
}

typedef struct RangeIterator {
    Range range;
    py_i64 current;
} RangeIterator;

static bool range_iterator__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_range);
    RangeIterator* ud = py_newobject(py_retval(), tp_range_iterator, 0, sizeof(RangeIterator));
    ud->range = *(Range*)py_touserdata(py_arg(1));
    ud->current = ud->range.start;
    return true;
}

bool range_iterator__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    RangeIterator* ud = py_touserdata(argv);
    if(ud->range.step > 0) {
        if(ud->current >= ud->range.stop) return StopIteration();
    } else {
        if(ud->current <= ud->range.stop) return StopIteration();
    }
    py_newint(py_retval(), ud->current);
    ud->current += ud->range.step;
    return true;
}

py_Type pk_range_iterator__register() {
    py_Type type = pk_newtype("range_iterator", tp_object, NULL, NULL, false, true);

    py_bindmagic(type, __new__, range_iterator__new__);
    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, range_iterator__next__);
    return type;
}