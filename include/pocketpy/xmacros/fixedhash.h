#if !defined(FIXEDHASH_T__HEADER) && !defined(FIXEDHASH_T__SOURCE)

#include "pocketpy/common/chunkedvector.h"
#include "pocketpy/config.h"
#include <stdint.h>

#define FIXEDHASH_T__HEADER
#define FIXEDHASH_T__SOURCE
/* Input */
#define K int
#define V float
#define NAME c11_fixedhash_d2f
#endif

/* Optional Input */
#ifndef hash
#define hash(a) ((uint64_t)(a))
#endif

#ifndef equal
#define equal(a, b) ((a) == (b))
#endif

/* Temporary macros */
#define CONCAT(A, B) CONCAT_(A, B)
#define CONCAT_(A, B) A##B

#define KV CONCAT(NAME, _KV)
#define METHOD(name) CONCAT(NAME, CONCAT(__, name))

#ifdef FIXEDHASH_T__HEADER
/* Declaration */
typedef struct {
    uint64_t hash;
    K key;
    V val;
} KV;

typedef struct {
    int length;
    uint16_t indices[0x10000];
    c11_chunkedvector /*T=FixedHashEntry*/ entries;
} NAME;

void METHOD(ctor)(NAME* self);
void METHOD(dtor)(NAME* self);
NAME* METHOD(new)();
void METHOD(delete)(NAME* self);
void METHOD(set)(NAME* self, K key, V* value);
V* METHOD(try_get)(NAME* self, K key);
bool METHOD(contains)(NAME* self, K key);

#endif

#ifdef FIXEDHASH_T__SOURCE
/* Implementation */

void METHOD(ctor)(NAME* self) {
    self->length = 0;
    memset(self->indices, 0xFF, sizeof(self->indices));
    c11_chunkedvector__ctor(&self->entries, sizeof(KV), 0);
}

void METHOD(dtor)(NAME* self) { c11_chunkedvector__dtor(&self->entries); }

NAME* METHOD(new)() {
    NAME* self = PK_MALLOC(sizeof(NAME));
    METHOD(ctor)(self);
    return self;
}

void METHOD(delete)(NAME* self) {
    METHOD(dtor)(self);
    PK_FREE(self);
}

void METHOD(set)(NAME* self, K key, V* value) {
    uint64_t hash_value = hash(key);
    int index = (uint16_t)(hash_value & 0xFFFF);
    while(self->indices[index] != 0xFFFF) {
        KV* entry = c11_chunkedvector__at(&self->entries, self->indices[index]);
        if(equal(entry->key, key)) {
            entry->val = *value;
            return;
        }
        index = ((5 * index) + 1) & 0xFFFF;
    }
    if(self->length >= 65000) abort();
    KV* kv = c11_chunkedvector__emplace(&self->entries);
    kv->hash = hash_value;
    kv->key = key;
    kv->val = *value;
    self->indices[index] = self->entries.length - 1;
    self->length++;
}

V* METHOD(try_get)(NAME* self, K key) {
    uint64_t hash_value = hash(key);
    int index = (uint16_t)(hash_value & 0xFFFF);
    while(self->indices[index] != 0xFFFF) {
        KV* entry = c11_chunkedvector__at(&self->entries, self->indices[index]);
        if(equal(entry->key, key)) return &entry->val;
        index = ((5 * index) + 1) & 0xFFFF;
    }
    return NULL;
}

bool METHOD(contains)(NAME* self, K key) {
    V* value = METHOD(try_get)(self, key);
    return value != NULL;
}

#endif

/* Undefine all macros */
#undef KV
#undef METHOD
#undef CONCAT
#undef CONCAT_

#undef K
#undef V
#undef NAME
#undef less
#undef partial_less
#undef equal
#undef hash
