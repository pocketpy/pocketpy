#include "pocketpy/common/threads.h"
#include <stdio.h>

int64_t time_ns();

static void func(void* arg) {
    long long* val = (long long*)arg;
    long long sum = 0;
    for(int i = 0; i < 100000; i++) {
        sum += *val;
    }
    *val = sum;
}

int main(int argc, char** argv) {
    int threads_num = 16;
    if(argc == 2) threads_num = atoi(argv[1]);
    printf("Using %d threads in the thread pool.\n", threads_num);

    c11_thrdpool pool;
    c11_thrdpool__ctor(&pool, threads_num);

    int num_tasks = 10000;
    long long* data = PK_MALLOC(sizeof(long long) * num_tasks);
    void** args = PK_MALLOC(sizeof(void*) * num_tasks);

    for(int i = 0; i < 10; i++) {
        for(int i = 0; i < num_tasks; i++) {
            data[i] = i;
            args[i] = &data[i];
        }

        printf("==> %dth run\n", i + 1);
        int64_t start_ns = time_ns();
        c11_thrdpool__map(&pool, func, args, num_tasks);
        int64_t end_ns = time_ns();
        double elapsed = (end_ns - start_ns) / 1e9;
        printf("  Results: %lld, %lld, %lld, %lld, %lld\n",
               data[0],
               data[1],
               data[2],
               data[100],
               data[400]);
        printf("  Elapsed time for %d tasks: %.6f seconds\n", num_tasks, elapsed);
        for(int i = 0; i < 5000000; i++) {
            c11_thrd__yield();
        }
    }

    c11_thrdpool__dtor(&pool);
    PK_FREE(args);
    PK_FREE(data);
    return 0;
}