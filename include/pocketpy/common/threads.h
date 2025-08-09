#pragma once

#include "pocketpy/config.h"

#if PK_ENABLE_THREADS

#include <stdatomic.h>
#include <stdbool.h>

#if __EMSCRIPTEN__ || __APPLE__ || __linux__
#include <pthread.h>
#define PK_USE_PTHREADS 1
typedef pthread_t c11_thrd_t;
typedef void* c11_thrd_retval_t;
#else
#include <threads.h>
#define PK_USE_PTHREADS 0
typedef thrd_t c11_thrd_t;
typedef int c11_thrd_retval_t;
#endif

bool c11_thrd_create(c11_thrd_t* thrd, c11_thrd_retval_t (*func)(void*), void* arg);
void c11_thrd_yield();

#endif