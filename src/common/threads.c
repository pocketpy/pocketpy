#include "pocketpy/common/threads.h"
#include "pocketpy/common/utils.h"
#include <stdarg.h>

#if PK_ENABLE_THREADS

#if PK_USE_PTHREADS

bool c11_thrd__create(c11_thrd_t* thrd, c11_thrd_func_t func, void* arg) {
    int res = pthread_create(thrd, NULL, func, arg);
    return res == 0;
}

void c11_thrd__yield() { sched_yield(); }

void c11_thrd__join(c11_thrd_t thrd) { pthread_join(thrd, NULL); }

c11_thrd_t c11_thrd__current() { return pthread_self(); }

bool c11_thrd__equal(c11_thrd_t a, c11_thrd_t b) { return pthread_equal(a, b); }

void c11_mutex__ctor(c11_mutex_t* mutex) { pthread_mutex_init(mutex, NULL); }

void c11_mutex__dtor(c11_mutex_t* mutex) { pthread_mutex_destroy(mutex); }

void c11_mutex__lock(c11_mutex_t* mutex) { pthread_mutex_lock(mutex); }

void c11_mutex__unlock(c11_mutex_t* mutex) { pthread_mutex_unlock(mutex); }

void c11_cond__ctor(c11_cond_t* cond) { pthread_cond_init(cond, NULL); }

void c11_cond__dtor(c11_cond_t* cond) { pthread_cond_destroy(cond); }

void c11_cond__wait(c11_cond_t* cond, c11_mutex_t* mutex) { pthread_cond_wait(cond, mutex); }

void c11_cond__signal(c11_cond_t* cond) { pthread_cond_signal(cond); }

void c11_cond__broadcast(c11_cond_t* cond) { pthread_cond_broadcast(cond); }

#else

bool c11_thrd__create(c11_thrd_t* thrd, c11_thrd_func_t func, void* arg) {
    int res = thrd_create(thrd, func, arg);
    return res == thrd_success;
}

void c11_thrd__yield() { thrd_yield(); }

void c11_thrd__join(c11_thrd_t thrd) { thrd_join(thrd, NULL); }

c11_thrd_t c11_thrd__current() { return thrd_current(); }

bool c11_thrd__equal(c11_thrd_t a, c11_thrd_t b) { return thrd_equal(a, b); }

void c11_mutex__ctor(c11_mutex_t* mutex) { mtx_init(mutex, mtx_plain); }

void c11_mutex__dtor(c11_mutex_t* mutex) { mtx_destroy(mutex); }

void c11_mutex__lock(c11_mutex_t* mutex) { mtx_lock(mutex); }

void c11_mutex__unlock(c11_mutex_t* mutex) { mtx_unlock(mutex); }

void c11_cond__ctor(c11_cond_t* cond) { cnd_init(cond); }

void c11_cond__dtor(c11_cond_t* cond) { cnd_destroy(cond); }

void c11_cond__wait(c11_cond_t* cond, c11_mutex_t* mutex) { cnd_wait(cond, mutex); }

void c11_cond__signal(c11_cond_t* cond) { cnd_signal(cond); }

void c11_cond__broadcast(c11_cond_t* cond) { cnd_broadcast(cond); }

#endif

#define C11_THRDPOOL_DEBUG 0

#if C11_THRDPOOL_DEBUG
int64_t time_ns();

static void c11_thrdpool_debug_log(int index, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[512];
    int n = sprintf(buf, "[%.6f - Worker %2d] ", time_ns() / 1e9, index);
    vsprintf(buf + n, format, args);
    printf("%s\n", buf);
    va_end(args);
}
#else
#define c11_thrdpool_debug_log(...)                                                                \
    do {                                                                                           \
    } while(0)
#endif

static c11_thrd_retval_t _thrdpool_worker(void* arg) {
    c11_thrdpool_worker* p_worker = (c11_thrdpool_worker*)arg;
    c11_thrdpool_tasks* p_tasks = p_worker->p_tasks;

    while(true) {
        c11_thrdpool_debug_log(p_worker->index, "Waiting for mutex lock...");
        c11_mutex__lock(p_worker->p_mutex);
        atomic_fetch_add(p_worker->p_ready_workers_num, 1);
        c11_thrdpool_debug_log(p_worker->index, "Mutex locked, checking for tasks...");
        if(atomic_load(&p_tasks->sync_val) == -1) {
            c11_mutex__unlock(p_worker->p_mutex);
            return 0;  // force kill
        }
        while(true) {
            c11_cond__wait(p_worker->p_cond, p_worker->p_mutex);
            int sync_val = atomic_load(&p_tasks->sync_val);
            c11_thrdpool_debug_log(p_worker->index,
                                   "Woke up from condition variable, sync_val=%d",
                                   sync_val);
            if(sync_val == 1) break;
            if(sync_val == -1) {
                c11_mutex__unlock(p_worker->p_mutex);
                return 0;  // force kill
            }
        }
        c11_mutex__unlock(p_worker->p_mutex);
        atomic_fetch_sub(p_worker->p_ready_workers_num, 1);

        c11_thrdpool_debug_log(p_worker->index, "Received tasks, starting execution...");
        // execute tasks
        while(true) {
            int arg_index = atomic_fetch_add(&p_tasks->current_index, 1);
            if(arg_index < p_tasks->length) {
                void* arg = p_tasks->args[arg_index];
                p_tasks->func(arg);
                atomic_fetch_add(&p_tasks->completed_count, 1);
            } else {
                break;
            }
        }

        c11_thrdpool_debug_log(p_worker->index,
                               "Execution complete, waiting for `sync_val` reset...");
        while(true) {
            int sync_val = atomic_load(&p_tasks->sync_val);
            if(sync_val == 0) break;
            if(sync_val == -1) return 0;  // force kill
            c11_thrd__yield();
        }
        c11_thrdpool_debug_log(p_worker->index,
                               "`sync_val` reset detected, waiting for next tasks...");
    }
    return 0;
}

void c11_thrdpool__ctor(c11_thrdpool* pool, int length) {
    pool->length = length;
    atomic_store(&pool->ready_workers_num, 0);
    pool->workers = PK_MALLOC(sizeof(c11_thrdpool_worker) * length);

    c11_mutex__ctor(&pool->workers_mutex);
    c11_cond__ctor(&pool->workers_cond);

    atomic_store(&pool->tasks.sync_val, 0);

    for(int i = 0; i < length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        p_worker->index = i;
        p_worker->p_ready_workers_num = &pool->ready_workers_num;
        p_worker->p_mutex = &pool->workers_mutex;
        p_worker->p_cond = &pool->workers_cond;
        p_worker->p_tasks = &pool->tasks;
        bool ok = c11_thrd__create(&p_worker->thread, _thrdpool_worker, p_worker);
        c11__rtassert(ok);
    }
}

void c11_thrdpool__dtor(c11_thrdpool* pool) {
    c11_mutex__lock(&pool->workers_mutex);
    atomic_store(&pool->tasks.sync_val, -1);
    c11_thrdpool_debug_log(-1, "Terminating all workers...");
    c11_cond__broadcast(&pool->workers_cond);
    c11_mutex__unlock(&pool->workers_mutex);

    for(int i = 0; i < pool->length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        c11_thrd__join(p_worker->thread);
    }

    c11_mutex__dtor(&pool->workers_mutex);
    c11_cond__dtor(&pool->workers_cond);
    PK_FREE(pool->workers);
}

void c11_thrdpool__map(c11_thrdpool* pool, c11_thrdpool_func_t func, void** args, int num_tasks) {
    if(num_tasks == 0) return;
    c11_thrdpool_debug_log(-1, "c11_thrdpool__map() called on %d tasks...", num_tasks);
    while(atomic_load(&pool->ready_workers_num) < pool->length) {
        c11_thrd__yield();
    }
    c11_thrdpool_debug_log(-1, "All %d workers are ready.", pool->length);

    // assign tasks
    c11_mutex__lock(&pool->workers_mutex);
    pool->tasks.func = func;
    pool->tasks.args = args;
    pool->tasks.length = num_tasks;
    atomic_store(&pool->tasks.sync_val, 1);
    atomic_store(&pool->tasks.current_index, 0);
    atomic_store(&pool->tasks.completed_count, 0);
    c11_cond__broadcast(&pool->workers_cond);
    c11_mutex__unlock(&pool->workers_mutex);
    // wait for complete
    c11_thrdpool_debug_log(-1, "Waiting for %d tasks to complete...", num_tasks);
    while(atomic_load(&pool->tasks.completed_count) < num_tasks) {
        c11_thrd__yield();
    }
    atomic_store(&pool->tasks.sync_val, 0);
    c11_thrdpool_debug_log(-1, "All %d tasks completed, `sync_val` was reset.", num_tasks);
}

#endif  // PK_ENABLE_THREADS