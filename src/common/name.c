#if PK_ENABLE_CUSTOM_SNAME == 0

#include "pocketpy/common/name.h"
#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/common/threads.h"
#include <assert.h>

typedef struct NameBucket NameBucket;

typedef struct NameBucket {
    NameBucket* next;
    uint64_t hash;
    int size;     // size of the data excluding the null-terminator
    char data[];  // null-terminated data
} NameBucket;

static struct {
    NameBucket* table[0x10000];
#if PK_ENABLE_THREADS
    atomic_flag lock;
#endif
} pk_string_table;

#define MAGIC_METHOD(x) py_Name x;
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD

void pk_names_initialize() {
#define MAGIC_METHOD(x) x = py_name(#x);
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD
}

void pk_names_finalize() {
    for(int i = 0; i < 0x10000; i++) {
        NameBucket* p = pk_string_table.table[i];
        while(p) {
            NameBucket* next = p->next;
            PK_FREE(p);
            p = next;
        }
        pk_string_table.table[i] = NULL;
    }
}

py_Name py_namev(c11_sv name) {
#if PK_ENABLE_THREADS
    while(atomic_flag_test_and_set(&pk_string_table.lock)) {
        c11_thrd__yield();
    }
#endif
    uint64_t hash = c11_sv__hash(name);
    int index = hash & 0xFFFF;
    NameBucket* p = pk_string_table.table[index];
    NameBucket* prev = NULL;
    bool found = false;
    while(p) {
        c11_sv p_sv = {p->data, p->size};
        if(p->hash == hash && c11__sveq(p_sv, name)) {
            found = true;
            break;
        }
        prev = p;
        p = p->next;
    }
    if(found) {
#if PK_ENABLE_THREADS
        atomic_flag_clear(&pk_string_table.lock);
#endif
        return (py_Name)p;
    }

    // generate new index
    NameBucket* bucket = PK_MALLOC(sizeof(NameBucket) + name.size + 1);
    bucket->next = NULL;
    bucket->hash = hash;
    bucket->size = name.size;
    memcpy(bucket->data, name.data, name.size);
    bucket->data[name.size] = '\0';
    if(prev == NULL) {
        pk_string_table.table[index] = bucket;
    } else {
        assert(prev->next == NULL);
        prev->next = bucket;
    }
#if PK_ENABLE_THREADS
    atomic_flag_clear(&pk_string_table.lock);
#endif
    return (py_Name)bucket;
}

c11_sv py_name2sv(py_Name index) {
    NameBucket* p = (NameBucket*)index;
    return (c11_sv){p->data, p->size};
}

py_Name py_name(const char* name) {
    c11_sv sv;
    sv.data = name;
    sv.size = strlen(name);
    return py_namev(sv);
}

const char* py_name2str(py_Name index) {
    NameBucket* p = (NameBucket*)index;
    return p->data;
}

#endif