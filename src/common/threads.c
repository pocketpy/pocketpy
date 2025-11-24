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

#endif

static c11_thrd_retval_t _thrdpool_worker(void* arg) {
    c11_thrdpool_worker* p_worker = (c11_thrdpool_worker*)arg;
    while(true) {
        c11_mutex__lock(&p_worker->mutex);
        while(p_worker->func == NULL && !p_worker->should_exit) {
            c11_cond__wait(&p_worker->cond, &p_worker->mutex);
        }

        if(p_worker->should_exit) {
            c11_mutex__unlock(&p_worker->mutex);
            break;
        }

        c11_thrd_func_t func = p_worker->func;
        void* arg = p_worker->arg;
        p_worker->func = NULL;
        p_worker->arg = NULL;
        c11_mutex__unlock(&p_worker->mutex);

        func(arg);
    }
    return 0;
}

void c11_thrdpool__ctor(c11_thrdpool* pool, int length) {
    pool->length = length;
    pool->workers = PK_MALLOC(sizeof(c11_thrdpool_worker) * length);
    pool->main_thread = c11_thrd__current();
    for(int i = 0; i < length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];

        c11_mutex__ctor(&p_worker->mutex);
        c11_cond__ctor(&p_worker->cond);
        p_worker->func = NULL;
        p_worker->arg = NULL;
        p_worker->should_exit = false;

        bool ok = c11_thrd__create(&p_worker->thread, _thrdpool_worker, p_worker);
        c11__rtassert(ok);
    }
}

void c11_thrdpool__dtor(c11_thrdpool* pool) {
    for(int i = 0; i < pool->length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        c11_mutex__lock(&p_worker->mutex);
        p_worker->should_exit = true;
        c11_cond__signal(&p_worker->cond);
        c11_mutex__unlock(&p_worker->mutex);
    }

    for(int i = 0; i < pool->length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        c11_thrd__join(p_worker->thread);
        c11_mutex__dtor(&p_worker->mutex);
        c11_cond__dtor(&p_worker->cond);
    }

    PK_FREE(pool->workers);
}

bool c11_thrdpool__create(c11_thrdpool* pool, c11_thrd_func_t func, void* arg) {
    // must be called from the main thread
    c11_thrd_t curr_thread = c11_thrd__current();
    c11__rtassert(c11_thrd__equal(curr_thread, pool->main_thread));

    // find the 1st idle worker
    for(int i = 0; i < pool->length; i++) {
        c11_thrdpool_worker* p_worker = &pool->workers[i];
        c11_mutex__lock(&p_worker->mutex);
        if(p_worker->func == NULL) {
            p_worker->func = func;
            p_worker->arg = arg;
            c11_cond__signal(&p_worker->cond);
            c11_mutex__unlock(&p_worker->mutex);
            return true;  // Task assigned
        }
        c11_mutex__unlock(&p_worker->mutex);
    }
    return false;  // no idle worker found
}

#endif  // PK_ENABLE_THREADS