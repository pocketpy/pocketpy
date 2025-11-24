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
typedef pthread_mutex_t c11_mutex_t;
typedef pthread_cond_t c11_cond_t;
#else
#include <threads.h>
#define PK_USE_PTHREADS 0
typedef thrd_t c11_thrd_t;
typedef int c11_thrd_retval_t;
typedef mtx_t c11_mutex_t;
typedef cnd_t c11_cond_t;
#endif

typedef c11_thrd_retval_t (*c11_thrd_func_t)(void*);

bool c11_thrd__create(c11_thrd_t* thrd, c11_thrd_func_t func, void* arg);
void c11_thrd__yield();
void c11_thrd__join(c11_thrd_t thrd);
c11_thrd_t c11_thrd__current();
bool c11_thrd__equal(c11_thrd_t a, c11_thrd_t b);

void c11_mutex__ctor(c11_mutex_t* mutex);
void c11_mutex__dtor(c11_mutex_t* mutex);
void c11_mutex__lock(c11_mutex_t* mutex);
void c11_mutex__unlock(c11_mutex_t* mutex);

void c11_cond__ctor(c11_cond_t* cond);
void c11_cond__dtor(c11_cond_t* cond);
void c11_cond__wait(c11_cond_t* cond, c11_mutex_t* mutex);
void c11_cond__signal(c11_cond_t* cond);
void c11_cond__broadcast(c11_cond_t* cond);

typedef void (*c11_thrdpool_func_t)(void* arg);

typedef struct c11_thrdpool_tasks {
    atomic_int sync_val;
    c11_thrdpool_func_t func;
    void** args;
    int length;
    atomic_int current_index;
    atomic_int completed_count;
} c11_thrdpool_tasks;

typedef struct c11_thrdpool_worker {
    int index;
    atomic_int* p_ready_workers_num;
    c11_mutex_t* p_mutex;
    c11_cond_t* p_cond;
    c11_thrdpool_tasks* p_tasks;
    c11_thrd_t thread;
} c11_thrdpool_worker;

typedef struct c11_thrdpool {
    int length;
    atomic_int ready_workers_num;

    c11_thrdpool_worker* workers;

    c11_mutex_t workers_mutex;
    c11_cond_t workers_cond;
    c11_thrdpool_tasks tasks;
} c11_thrdpool;

void c11_thrdpool__ctor(c11_thrdpool* pool, int length);
void c11_thrdpool__dtor(c11_thrdpool* pool);
void c11_thrdpool__map(c11_thrdpool* pool, c11_thrdpool_func_t func, void** args, int num_tasks);

#endif