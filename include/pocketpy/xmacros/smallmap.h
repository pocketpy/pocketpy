#pragma once

#if !defined(SMALLMAP_T__HEADER) && !defined(SMALLMAP_T__SOURCE)
    #include "pocketpy/common/vector.h"

    /* Input */
    #define K int
    #define V float
#endif

/* Optional Input */
#ifndef less
    #define less(a, b) ((a.key) < (b))
#endif

/* Temprary macros */
#define CONCAT(A, B) CONCAT_(A, B)
#define CONCAT_(A, B) A##B

#define KV CONCAT(CONCAT(c11_smallmap_entry_, K), CONCAT(_, V))
#define SMALLMAP CONCAT(CONCAT(c11_smallmap_, K), CONCAT(_, V))
#define SMALLMAP_METHOD(name) CONCAT(SMALLMAP, CONCAT(__, name))

typedef struct {
    K key;
    V value;
} KV;

typedef struct{
    c11_vector entries;
} SMALLMAP;

void SMALLMAP_METHOD(ctor)(SMALLMAP* self);
void SMALLMAP_METHOD(dtor)(SMALLMAP* self);
void SMALLMAP_METHOD(set)(SMALLMAP* self, K key, V value);
V* SMALLMAP_METHOD(try_get)(SMALLMAP* self, K key);
V SMALLMAP_METHOD(get)(SMALLMAP* self, K key, V default_value);
bool SMALLMAP_METHOD(contains)(SMALLMAP* self, K key);
bool SMALLMAP_METHOD(del)(SMALLMAP* self, K key);
void SMALLMAP_METHOD(clear)(SMALLMAP* self);

#ifdef SMALLMAP_T__SOURCE
/* Implementation */

void SMALLMAP_METHOD(ctor)(SMALLMAP* self) {
    c11_vector__ctor(&self->entries, sizeof(KV));
}

void SMALLMAP_METHOD(dtor)(SMALLMAP* self) {
    c11_vector__dtor(&self->entries);
}

void SMALLMAP_METHOD(set)(SMALLMAP* self, K key, V value) {
    int index;
    c11__lower_bound(KV, self->entries.data, self->entries.count, key, less, &index);
    KV* it = c11__at(KV, &self->entries, index);
    if(index != self->entries.count && it->key == key) {
        it->value = value;
    } else {
        KV kv = {key, value};
        c11_vector__insert(KV, &self->entries, index, kv);
    }
}

V* SMALLMAP_METHOD(try_get)(SMALLMAP* self, K key) {
    int index;
    c11__lower_bound(KV, self->entries.data, self->entries.count, key, less, &index);
    KV* it = c11__at(KV, &self->entries, index);
    if(index != self->entries.count && it->key == key) {
        return &it->value;
    } else {
        return NULL;
    }
}

V SMALLMAP_METHOD(get)(SMALLMAP* self, K key, V default_value) {
    V* p = SMALLMAP_METHOD(try_get)(self, key);
    return p ? *p : default_value;
}

bool SMALLMAP_METHOD(contains)(SMALLMAP* self, K key) {
    return SMALLMAP_METHOD(try_get)(self, key) != NULL;
}

bool SMALLMAP_METHOD(del)(SMALLMAP* self, K key) {
    int index;
    c11__lower_bound(KV, self->entries.data, self->entries.count, key, less, &index);
    KV* it = c11__at(KV, &self->entries, index);
    if(index != self->entries.count && it->key == key) {
        c11_vector__erase(KV, &self->entries, index);
        return true;
    }
    return false;
}

void SMALLMAP_METHOD(clear)(SMALLMAP* self) {
    c11_vector__clear(&self->entries);
}

#endif

#undef KV
#undef SMALLMAP
#undef SMALLMAP_METHOD
#undef CONCAT
#undef CONCAT_

#undef K
#undef V
#undef less
