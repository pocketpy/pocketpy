#include "pocketpy/common/threads.h"

#if PK_ENABLE_THREADS

#if PK_USE_PTHREADS

bool c11_thrd_create(c11_thrd_t* thrd, c11_thrd_retval_t (*func)(void*), void* arg) {
    int res = pthread_create(thrd, NULL, func, arg);
    return res == 0;
}

void c11_thrd_yield() { sched_yield(); }

#else

bool c11_thrd_create(c11_thrd_t* thrd, c11_thrd_retval_t (*func)(void*), void* arg) {
    int res = thrd_create(thrd, func, arg);
    return res == thrd_success;
}

void c11_thrd_yield() { thrd_yield(); }

#endif

#endif  // PK_ENABLE_THREADS