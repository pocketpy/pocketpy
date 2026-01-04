#define _POSIX_C_SOURCE 199309L
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define ARRAY_SIZE 1000
#define NUM_ROUNDS 100000

static inline uint64_t nanos(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static uint64_t bench_int(void) {
    int arr[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) arr[i] = 1;

    volatile long long total = 0;
    uint64_t start = nanos();
    for (int round = 0; round < NUM_ROUNDS; ++round) {
        long long sum = 0;
        for (int i = 0; i < ARRAY_SIZE; ++i) sum += arr[i];
        total += sum;
    }
    uint64_t end = nanos();
    // prevent optimization
    if (total == 0) printf("unused: %lld\n", total);
    return (end - start) / NUM_ROUNDS;
}

static uint64_t bench_char(void) {
    char arr[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; ++i) arr[i] = 1;

    volatile long long total = 0;
    uint64_t start = nanos();
    for (int round = 0; round < NUM_ROUNDS; ++round) {
        long long sum = 0;
        for (int i = 0; i < ARRAY_SIZE; ++i) sum += arr[i];
        total += sum;
    }
    uint64_t end = nanos();
    if (total == 0) printf("unused: %lld\n", total);
    return (end - start) / NUM_ROUNDS;
}

int main(void) {
    uint64_t int_avg = bench_int();
    uint64_t char_avg = bench_char();
    printf("int[%d] average traversal (ns): %llu\n", ARRAY_SIZE,
           (unsigned long long)int_avg);
    printf("char[%d] average traversal (ns): %llu\n", ARRAY_SIZE,
           (unsigned long long)char_avg);
    return 0;
}
