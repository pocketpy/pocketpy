#include "pocketpy/pocketpy.h"
#include "pocketpy/interpreter/vm.h"
#include <time.h>

#define NANOS_PER_SEC 1000000000

static bool get_ns(int64_t* out) {
    struct timespec tms;
#if defined( __ANDROID__) || defined(__MINGW32__) || defined(__MINGW64__)
    clock_gettime(CLOCK_REALTIME, &tms);
#else
    /* The C11 way */
    timespec_get(&tms, TIME_UTC);
#endif
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

static bool time_localtime(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    py_Type tp_struct_time = py_gettype("time", py_name("struct_time"));
    assert(tp_struct_time);
    struct tm* ud = py_newobject(py_retval(), tp_struct_time, 0, sizeof(struct tm));
    time_t t = time(NULL);
    *ud = *localtime(&t);
    return true;
}

#define DEF_STRUCT_TIME__PROPERTY(name, expr)                                                      \
    static bool struct_time__##name(int argc, py_Ref argv) {                                       \
        PY_CHECK_ARGC(1);                                                                          \
        struct tm* tm = py_touserdata(argv);                                                       \
        py_newint(py_retval(), expr);                                                              \
        return true;                                                                               \
    }

DEF_STRUCT_TIME__PROPERTY(tm_year, tm->tm_year + 1900)
DEF_STRUCT_TIME__PROPERTY(tm_mon, tm->tm_mon + 1)
DEF_STRUCT_TIME__PROPERTY(tm_mday, tm->tm_mday)
DEF_STRUCT_TIME__PROPERTY(tm_hour, tm->tm_hour)
DEF_STRUCT_TIME__PROPERTY(tm_min, tm->tm_min)
DEF_STRUCT_TIME__PROPERTY(tm_sec, tm->tm_sec)
DEF_STRUCT_TIME__PROPERTY(tm_wday, (tm->tm_wday + 6) % 7)
DEF_STRUCT_TIME__PROPERTY(tm_yday, tm->tm_yday + 1)
DEF_STRUCT_TIME__PROPERTY(tm_isdst, tm->tm_isdst)

#undef DEF_STRUCT_TIME__PROPERTY

void pk__add_module_time() {
    py_Ref mod = py_newmodule("time");

    py_Type tp_struct_time = py_newtype("struct_time", tp_object, mod, NULL);

    py_bindproperty(tp_struct_time, "tm_year", struct_time__tm_year, NULL);
    py_bindproperty(tp_struct_time, "tm_mon", struct_time__tm_mon, NULL);
    py_bindproperty(tp_struct_time, "tm_mday", struct_time__tm_mday, NULL);
    py_bindproperty(tp_struct_time, "tm_hour", struct_time__tm_hour, NULL);
    py_bindproperty(tp_struct_time, "tm_min", struct_time__tm_min, NULL);
    py_bindproperty(tp_struct_time, "tm_sec", struct_time__tm_sec, NULL);
    py_bindproperty(tp_struct_time, "tm_wday", struct_time__tm_wday, NULL);
    py_bindproperty(tp_struct_time, "tm_yday", struct_time__tm_yday, NULL);
    py_bindproperty(tp_struct_time, "tm_isdst", struct_time__tm_isdst, NULL);

    py_bindfunc(mod, "time", time_time);
    py_bindfunc(mod, "time_ns", time_time_ns);
    py_bindfunc(mod, "sleep", time_sleep);
    py_bindfunc(mod, "localtime", time_localtime);
}
