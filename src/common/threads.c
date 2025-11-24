#include "pocketpy/common/threads.h"
#include "pocketpy/common/utils.h"

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

static bool _thrdpool_worker_sync(c11_mutex_t* p_mutex,
                                  c11_cond_t* p_cond,
                                  int* p_sync_val,
                                  int expected_sync_val) {
    int index = (int)expected_sync_val;
    c11_mutex__lock(p_mutex + index);
    while(true) {
        c11_cond__wait(p_cond + index, p_mutex + index);
        if(*p_sync_val == -1) return false;
        if(*p_sync_val == expected_sync_val) break;
    }
    c11_mutex__unlock(p_mutex + index);
    return true;
}

static c11_thrd_retval_t _thrdpool_worker(void* arg) {
    c11_thrdpool_worker* p_worker = (c11_thrdpool_worker*)arg;
    c11_thrdpool_tasks* p_tasks = p_worker->p_tasks;

    while(true) {
        if(!_thrdpool_worker_sync(p_worker->p_mutex, p_worker->p_cond, &p_tasks->sync_val, 0)) {
            break;
        }

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

        if(!_thrdpool_worker_sync(p_worker->p_mutex, p_worker->p_cond, &p_tasks->sync_val, 1)) {
            break;
        }
    }
    return 0;
}

void c11_thrdpool__ctor(c11_thrdpool* pool, int length) {
    pool->length = length;
    pool->workers = PK_MALLOC(sizeof(c11_thrdpool_worker) * length);
    atomic_store(&pool->is_busy, false);

    c11_mutex__ctor(&pool->workers_mutex[0]);
    c11_mutex__ctor(&pool->workers_mutex[1]);
    c11_cond__ctor(&pool->workers_cond[0]);
    c11_cond__ctor(&pool->workers_cond[1]);

    for(int i = 0; i < length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        p_worker->p_mutex = pool->workers_mutex;
        p_worker->p_cond = pool->workers_cond;
        p_worker->p_tasks = &pool->tasks;
        bool ok = c11_thrd__create(&p_worker->thread, _thrdpool_worker, p_worker);
        c11__rtassert(ok);
    }
}

void c11_thrdpool__dtor(c11_thrdpool* pool) {
    c11_mutex__lock(&pool->workers_mutex[0]);
    c11_mutex__lock(&pool->workers_mutex[1]);
    pool->tasks.sync_val = -1;
    c11_mutex__unlock(&pool->workers_mutex[1]);
    c11_mutex__unlock(&pool->workers_mutex[0]);
    
    c11_cond__broadcast(&pool->workers_cond[0]);
    c11_cond__broadcast(&pool->workers_cond[1]);

    for(int i = 0; i < pool->length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        c11_thrd__join(p_worker->thread);
    }

    PK_FREE(pool->workers);
    c11_mutex__dtor(&pool->workers_mutex[0]);
    c11_mutex__dtor(&pool->workers_mutex[1]);
    c11_cond__dtor(&pool->workers_cond[0]);
    c11_cond__dtor(&pool->workers_cond[1]);
}

void c11_thrdpool__map(c11_thrdpool* pool, c11_thrdpool_func_t func, void** args, int num_tasks) {
    if(num_tasks == 0) return;
    bool expected = false;
    while(!atomic_compare_exchange_weak(&pool->is_busy, &expected, true)) {
        expected = false;
        c11_thrd__yield();
    }
    // assign tasks
    c11_mutex__lock(&pool->workers_mutex[0]);
    pool->tasks.func = func;
    pool->tasks.args = args;
    pool->tasks.length = num_tasks;
    pool->tasks.sync_val = 0;
    atomic_store(&pool->tasks.current_index, 0);
    atomic_store(&pool->tasks.completed_count, 0);
    c11_cond__broadcast(&pool->workers_cond[0]);
    c11_mutex__unlock(&pool->workers_mutex[0]);
    // wait for complete
    while(atomic_load(&pool->tasks.completed_count) < num_tasks) {
        c11_thrd__yield();
    }
    // notify workers to proceed
    c11_mutex__lock(&pool->workers_mutex[1]);
    pool->tasks.sync_val = 1;
    c11_cond__broadcast(&pool->workers_cond[1]);
    c11_mutex__unlock(&pool->workers_mutex[1]);
    // mark as not busy
    atomic_store(&pool->is_busy, false);
}

#endif  // PK_ENABLE_THREADS