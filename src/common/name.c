#include "pocketpy/common/name.h"
#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"
#include <stdatomic.h>

typedef struct NameBucket NameBucket;

typedef struct NameBucket {
    NameBucket* next;
    uint64_t hash;
    int size;     // size of the data excluding the null-terminator
    char data[];  // null-terminated data
} NameBucket;

static struct {
    NameBucket* table[0x10000];
    atomic_bool lock;
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
    while(atomic_exchange(&pk_string_table.lock, true)) {
        // busy-wait until the lock is released
    }
    uint64_t hash = c11_sv__hash(name);
    int index = hash & 0xFFFF;
    NameBucket* p = pk_string_table.table[index];
    bool found = false;
    while(p) {
        c11_sv p_sv = {p->data, p->size};
        if(p->hash == hash && c11__sveq(p_sv, name)) {
            found = true;
            break;
        } else {
            p = p->next;
        }
    }
    if(found) {
        atomic_store(&pk_string_table.lock, false);
        return (py_Name)p;
    }

    // generate new index
    NameBucket* bucket = PK_MALLOC(sizeof(NameBucket) + name.size + 1);
    bucket->next = NULL;
    bucket->hash = hash;
    bucket->size = name.size;
    memcpy(bucket->data, name.data, name.size);
    bucket->data[name.size] = '\0';
    if(p == NULL) {
        pk_string_table.table[index] = bucket;
    } else {
        assert(p->next == NULL);
        p->next = bucket;
    }
    atomic_store(&pk_string_table.lock, false);
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