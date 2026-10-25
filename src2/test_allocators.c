#include "pocketpy.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                                          \
    do {                                                                                          \
        if(!(condition)) {                                                                        \
            fprintf(stderr, "line %d: %s\n", __LINE__, #condition);                                \
            exit(1);                                                                              \
        }                                                                                         \
    } while(0)

typedef union Allocation {
    long double alignment;
    void* pointer_alignment;
    struct {
        size_t size;
        uint64_t magic;
    } info;
} Allocation;

static const uint64_t allocation_magic = UINT64_C(0x58df376bac2401e9);
static size_t malloc_calls, realloc_calls, free_calls, live_allocations;

static void* tracked_malloc(size_t size) {
    malloc_calls++;
    if(size > SIZE_MAX - sizeof(Allocation)) return NULL;
    Allocation* block = malloc(sizeof(Allocation) + size);
    if(!block) return NULL;
    block->info.size = size;
    block->info.magic = allocation_magic;
    live_allocations++;
    return block + 1;
}

static void tracked_free(void* ptr) {
    free_calls++;
    if(!ptr) return;
    Allocation* block = (Allocation*)ptr - 1;
    CHECK(block->info.magic == allocation_magic);
    block->info.magic = 0;
    CHECK(live_allocations > 0);
    live_allocations--;
    free(block);
}

static void* tracked_realloc(void* ptr, size_t size) {
    realloc_calls++;
    if(!ptr) return tracked_malloc(size);
    if(size == 0) {
        tracked_free(ptr);
        return NULL;
    }
    Allocation* block = (Allocation*)ptr - 1;
    CHECK(block->info.magic == allocation_magic);
    if(size > SIZE_MAX - sizeof(Allocation)) return NULL;
    block = realloc(block, sizeof(Allocation) + size);
    if(!block) return NULL;
    block->info.size = size;
    return block + 1;
}

int main(void) {
    // Default allocators work before a VM or callback accessor has been used.
    void* default_buffer = py_malloc(8);
    CHECK(default_buffer != NULL);
    default_buffer = py_realloc(default_buffer, 16);
    CHECK(default_buffer != NULL);
    py_free(default_buffer);

    py_AppCallbacks* callbacks = py_appcallbacks();
    CHECK(callbacks->malloc != NULL && callbacks->realloc != NULL && callbacks->free != NULL);
    callbacks->malloc = tracked_malloc;
    callbacks->realloc = tracked_realloc;
    callbacks->free = tracked_free;

    char* buffer = py_malloc(8);
    CHECK(buffer != NULL && malloc_calls == 1);
    memcpy(buffer, "pocketpy", 8);
    buffer = py_realloc(buffer, 64);
    CHECK(buffer != NULL && realloc_calls == 1);
    CHECK(memcmp(buffer, "pocketpy", 8) == 0);
    py_free(buffer);
    buffer = py_realloc(NULL, 8);
    CHECK(buffer != NULL);
    CHECK(py_realloc(buffer, 0) == NULL);
    py_free(NULL);
    CHECK(live_allocations == 0);

    size_t before_initialize = malloc_calls;
    py_initialize();
    CHECK(malloc_calls > before_initialize);
    CHECK(py_appcallbacks() == callbacks);
    CHECK(callbacks->malloc == tracked_malloc);
    CHECK(callbacks->realloc == tracked_realloc);
    CHECK(callbacks->free == tracked_free);
    CHECK(py_exec("values = [str(i) for i in range(1000)]\n"
                  "mapping = {i: values[i] for i in range(1000)}\n"
                  "assert len(mapping) == 1000\n",
                  "<allocator_test>", EXEC_MODE, NULL));

    CHECK(!py_exec("raise ValueError('allocator test')", "<allocator_error>", EXEC_MODE, NULL));
    char* message = py_formatexc();
    CHECK(message != NULL && strstr(message, "allocator test") != NULL);
    py_free(message);
    py_clearexc(NULL);

    py_switchvm(1);
    CHECK(py_exec("assert sum(range(100)) == 4950", "<allocator_vm>", EXEC_MODE, NULL));
    py_resetvm();
    py_switchvm(0);
    py_gc_collect();
    py_finalize();
    CHECK(live_allocations == 0);
    CHECK(malloc_calls > 1 && realloc_calls > 1 && free_calls > 1);
    puts("Allocator callback tests passed.");
    return 0;
}

#undef CHECK
