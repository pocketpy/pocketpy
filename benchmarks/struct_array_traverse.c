#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stdio.h>
#include <time.h>

static inline uint64_t nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t bench_int(void) {
    int arr[1000];
    for (int i = 0; i < 1000; ++i) arr[i] = 1;

    volatile long long total = 0;
    uint64_t start = nanos();
    for (int round = 0; round < 100000; ++round) {
        long long sum = 0;
        for (int i = 0; i < 1000; ++i) sum += arr[i];
        total += sum;
    }
    uint64_t end = nanos();
    // prevent optimization
    if (total == 0) printf("unused: %lld\n", total);
    return (end - start) / 100000;
}

static uint64_t bench_char(void) {
    char arr[1000];
    for (int i = 0; i < 1000; ++i) arr[i] = 1;

    volatile long long total = 0;
    uint64_t start = nanos();
    for (int round = 0; round < 100000; ++round) {
        long long sum = 0;
        for (int i = 0; i < 1000; ++i) sum += arr[i];
        total += sum;
    }
    uint64_t end = nanos();
    if (total == 0) printf("unused: %lld\n", total);
    return (end - start) / 100000;
}

int main(void) {
    uint64_t int_avg = bench_int();
    uint64_t char_avg = bench_char();
    printf("int[1000] average traversal (ns): %llu\n",
           (unsigned long long)int_avg);
    printf("char[1000] average traversal (ns): %llu\n",
           (unsigned long long)char_avg);
    return 0;
}
