#if !defined(SMALLMAP_T__HEADER) && !defined(SMALLMAP_T__SOURCE)
    #include "pocketpy/common/vector.h"

    #define SMALLMAP_T__HEADER
    #define SMALLMAP_T__SOURCE
    /* Input */
    #define K int
    #define V float
    #define NAME c11_smallmap_i2f
#endif

/* Optional Input */
#ifndef less
    #define less(a, b) ((a) < (b))
#endif

#ifndef equal
    #define equal(a, b) ((a) == (b))
#endif

/* Temprary macros */
#define partial_less(a, b)      less((a).key, (b))
#define CONCAT(A, B)            CONCAT_(A, B)
#define CONCAT_(A, B)           A##B

#define KV CONCAT(NAME, _KV)
#define METHOD(name) CONCAT(NAME, CONCAT(__, name))

#ifdef SMALLMAP_T__HEADER
/* Declaration */
typedef struct {
    K key;
    V value;
} KV;

typedef c11_vector NAME;

void METHOD(ctor)(NAME* self);
void METHOD(dtor)(NAME* self);
NAME* METHOD(new)();
void METHOD(delete)(NAME* self);
void METHOD(set)(NAME* self, K key, V value);
V* METHOD(try_get)(const NAME* self, K key);
V METHOD(get)(const NAME* self, K key, V default_value);
bool METHOD(contains)(const NAME* self, K key);
bool METHOD(del)(NAME* self, K key);
void METHOD(clear)(NAME* self);

#endif

#ifdef SMALLMAP_T__SOURCE
/* Implementation */

void METHOD(ctor)(NAME* self) {
    c11_vector__ctor(self, sizeof(KV));
    c11_vector__reserve(self, 4);
}

void METHOD(dtor)(NAME* self) {
    c11_vector__dtor(self);
}

NAME* METHOD(new)() {
    NAME* self = malloc(sizeof(NAME));
    METHOD(ctor)(self);
    return self;
}

void METHOD(delete)(NAME* self) {
    METHOD(dtor)(self);
    free(self);
}

void METHOD(set)(NAME* self, K key, V value) {
    int index;
    c11__lower_bound(KV, self->data, self->count, key, partial_less, &index);
    if(index != self->count){
        KV* it = c11__at(KV, self, index);
        if(equal(it->key, key)){
            it->value = value;
            return;
        }
    }
    KV kv = {key, value};
    c11_vector__insert(KV, self, index, kv);
}

V* METHOD(try_get)(const NAME* self, K key) {
    // use `bsearch` which is faster than `lower_bound`
    int low = 0;
    int high = self->count - 1;
    KV* a = self->data;
    while(low <= high){
        int mid = (low + high) / 2;
        if(equal(a[mid].key, key)){
            return &a[mid].value;
        } else if(less(a[mid].key, key)){
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return NULL;
}

V METHOD(get)(const NAME* self, K key, V default_value) {
    V* p = METHOD(try_get)(self, key);
    return p ? *p : default_value;
}

bool METHOD(contains)(const NAME* self, K key) {
    return METHOD(try_get)(self, key) != NULL;
}

bool METHOD(del)(NAME* self, K key) {
    int index;
    c11__lower_bound(KV, self->data, self->count, key, partial_less, &index);
    if(index != self->count){
        KV* it = c11__at(KV, self, index);
        if(equal(it->key, key)){
            c11_vector__erase(KV, self, index);
            return true;
        }
    }
    return false;
}

void METHOD(clear)(NAME* self) {
    c11_vector__clear(self);
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
