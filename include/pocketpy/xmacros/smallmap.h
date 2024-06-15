#if !defined(SMALLMAP_T__HEADER) && !defined(SMALLMAP_T__SOURCE)
    #include "pocketpy/common/vector.h"

    /* Input */
    #define K int
    #define V float
    #define TAG int_float
#endif

/* Optional Input */
#ifndef less
    #define less(a, b) ((a.key) < (b))
#endif

#ifndef equal
    #define equal(a, b) ((a) == (b))
#endif

/* Temprary macros */
#define CONCAT(A, B) CONCAT_(A, B)
#define CONCAT_(A, B) A##B

#define KV CONCAT(c11_smallmap_entry_, TAG)
#define SMALLMAP CONCAT(c11_smallmap_, TAG)
#define SMALLMAP_METHOD(name) CONCAT(SMALLMAP, CONCAT(__, name))

typedef struct {
    K key;
    V value;
} KV;

typedef c11_vector SMALLMAP;

void SMALLMAP_METHOD(ctor)(SMALLMAP* self);
void SMALLMAP_METHOD(dtor)(SMALLMAP* self);
void SMALLMAP_METHOD(set)(SMALLMAP* self, K key, V value);
V* SMALLMAP_METHOD(try_get)(const SMALLMAP* self, K key);
V SMALLMAP_METHOD(get)(const SMALLMAP* self, K key, V default_value);
bool SMALLMAP_METHOD(contains)(const SMALLMAP* self, K key);
bool SMALLMAP_METHOD(del)(SMALLMAP* self, K key);
void SMALLMAP_METHOD(clear)(SMALLMAP* self);

#ifdef SMALLMAP_T__SOURCE
/* Implementation */

void SMALLMAP_METHOD(ctor)(SMALLMAP* self) {
    c11_vector__ctor(self, sizeof(KV));
}

void SMALLMAP_METHOD(dtor)(SMALLMAP* self) {
    c11_vector__dtor(self);
}

void SMALLMAP_METHOD(set)(SMALLMAP* self, K key, V value) {
    int index;
    c11__lower_bound(KV, self->data, self->count, key, less, &index);
    KV* it = c11__at(KV, self, index);
    if(index != self->count && equal(it->key, key)) {
        it->value = value;
    } else {
        KV kv = {key, value};
        c11_vector__insert(KV, self, index, kv);
    }
}

V* SMALLMAP_METHOD(try_get)(const SMALLMAP* self, K key) {
    int index;
    c11__lower_bound(KV, self->data, self->count, key, less, &index);
    KV* it = c11__at(KV, self, index);
    if(index != self->count && equal(it->key, key)) {
        return &it->value;
    } else {
        return NULL;
    }
}

V SMALLMAP_METHOD(get)(const SMALLMAP* self, K key, V default_value) {
    V* p = SMALLMAP_METHOD(try_get)(self, key);
    return p ? *p : default_value;
}

bool SMALLMAP_METHOD(contains)(const SMALLMAP* self, K key) {
    return SMALLMAP_METHOD(try_get)(self, key) != NULL;
}

bool SMALLMAP_METHOD(del)(SMALLMAP* self, K key) {
    int index;
    c11__lower_bound(KV, self->data, self->count, key, less, &index);
    KV* it = c11__at(KV, self, index);
    if(index != self->count && equal(it->key, key)) {
        c11_vector__erase(KV, self, index);
        return true;
    }
    return false;
}

void SMALLMAP_METHOD(clear)(SMALLMAP* self) {
    c11_vector__clear(self);
}

#endif

#undef KV
#undef SMALLMAP
#undef SMALLMAP_METHOD
#undef CONCAT
#undef CONCAT_

#undef K
#undef V
#undef TAG
#undef less
#undef equal
