#include "pocketpy/pocketpy.h"
#include "time.h"
#include <stdint.h>

#define NANOS_PER_SEC 1000000000

static bool get_ns(int64_t* out) {
    struct timespec tms;
    /* The C11 way */
    if(!timespec_get(&tms, TIME_UTC)) {
        return py_exception(tp_OSError, "%s", "timespec_get() failed");
    }
    /* seconds, multiplied with 1 billion */
    int64_t nanos = tms.tv_sec * NANOS_PER_SEC;
    /* Add full nanoseconds */
    nanos += tms.tv_nsec;
    *out = nanos;
    return true;
}

static bool time_time(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int64_t nanos;
    if(!get_ns(&nanos)) return false;
    py_newfloat(py_retval(), (double)nanos / NANOS_PER_SEC);
    return true;
}

static bool time_time_ns(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    int64_t nanos;
    if(!get_ns(&nanos)) return false;
    py_newint(py_retval(), nanos);
    return true;
}

static bool time_sleep(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_f64 secs;
    if(!py_castfloat(argv, &secs)) return false;

    int64_t start;
    if(!get_ns(&start)) return false;
    const int64_t end = start + secs * 1000000000;
    while(true) {
        int64_t now;
        if(!get_ns(&now)) return false;
        if(now >= end) break;
    }
    py_newnone(py_retval());
    return true;
}

void pk__add_module_time() {
    py_Ref mod = py_newmodule("time");

    py_bindfunc(mod, "time", time_time);
    py_bindfunc(mod, "time_ns", time_time_ns);
    py_bindfunc(mod, "sleep", time_sleep);
}