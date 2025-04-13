#include "pocketpy.h"
#include "threads.h"
#include <stdio.h>

int run_huge_job_in_vm1(void* arg) {
    py_switchvm(1);
    bool ok = py_exec((const char*)arg, "<job>", EXEC_MODE, NULL);
    if(!ok) {
        py_printexc();
        return 1;
    }
    return 0;
}

int main() {
    py_initialize();

    bool ok = py_exec("print('Hello world from VM0!')", "<string1>", EXEC_MODE, NULL);
    if(!ok) {
        py_printexc();
        return 1;
    }

    printf("main vm index: %d\n", py_currentvm());

    char* job_string =
        "import time\n"
        "res = 0\n"
        "time.sleep(3)\n"
        "res = 100\n"
        "print('Huge job done!')\n"
        "print('Result:', res)\n";

    thrd_t thread1;
    thrd_create(&thread1, run_huge_job_in_vm1, job_string);

    for(int i = 0; i < 5; i++) {
        thrd_sleep(&(struct timespec){.tv_sec = 1, .tv_nsec = 0}, NULL);
        printf("main vm index: %d\n", py_currentvm());
    }

    int thrd_res;
    thrd_join(thread1, &thrd_res);
    printf("Thread result: %d\n", thrd_res);

    py_finalize();

    return 0;
}