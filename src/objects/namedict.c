#include "pocketpy/objects/namedict.h"
#include "pocketpy/common/utils.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>

// https://jfdube.wordpress.com/2011/10/12/hashing-strings-and-pointers-avoiding-common-pitfalls/
uintptr_t ThomasWangInt32Hash(void* Ptr) {
    // Here we think only the lower 32 bits are useful
    uint32_t Value = (uint32_t)(uintptr_t)Ptr;
    Value = ~Value + (Value << 15);
    Value = Value ^ (Value >> 12);
    Value = Value + (Value << 2);
    Value = Value ^ (Value >> 4);
    Value = Value * 2057;
    Value = Value ^ (Value >> 16);
    return Value;
}

#define HASH_KEY(__k) ThomasWangInt32Hash(__k)

#define HASH_PROBE_0(__k, ok, i)                                                                   \
    ok = false;                                                                                    \
    i = HASH_KEY(__k) & self->mask;                                                                \
    do {                                                                                           \
        if(self->items[i].key == (__k)) {                                                          \
            ok = true;                                                                             \
            break;                                                                                 \
        }                                                                                          \
        if(self->items[i].key == NULL) break;                                                      \
        i = (i + 1) & self->mask;                                                                  \
    } while(true);

#define HASH_PROBE_1(__k, ok, i)                                                                   \
    ok = false;                                                                                    \
    i = HASH_KEY(__k) & self->mask;                                                                \
    while(self->items[i].key != NULL) {                                                            \
        if(self->items[i].key == (__k)) {                                                          \
            ok = true;                                                                             \
            break;                                                                                 \
        }                                                                                          \
        i = (i + 1) & self->mask;                                                                  \
    }

static void NameDict__set_capacity_and_alloc_items(NameDict* self, int val) {
    self->capacity = val;
    self->critical_size = val * self->load_factor;
    self->mask = (uintptr_t)val - 1;

    self->items = PK_MALLOC(self->capacity * sizeof(NameDict_KV));
    memset(self->items, 0, self->capacity * sizeof(NameDict_KV));
}

static void NameDict__rehash_2x(NameDict* self) {
    NameDict_KV* old_items = self->items;
    int old_capacity = self->capacity;
    NameDict__set_capacity_and_alloc_items(self, self->capacity * 2);
    for(int i = 0; i < old_capacity; i++) {
        if(old_items[i].key == NULL) continue;
        bool ok;
        uintptr_t j;
        HASH_PROBE_1(old_items[i].key, ok, j);
        c11__rtassert(!ok);
        self->items[j] = old_items[i];
    }
    PK_FREE(old_items);
}

NameDict* NameDict__new(float load_factor) {
    NameDict* p = PK_MALLOC(sizeof(NameDict));
    NameDict__ctor(p, load_factor);
    return p;
}

void NameDict__delete(NameDict* self) {
    NameDict__dtor(self);
    PK_FREE(self);
}

void NameDict__ctor(NameDict* self, float load_factor) {
    assert(load_factor > 0.0f && load_factor < 1.0f);
    self->length = 0;
    self->load_factor = load_factor;
    NameDict__set_capacity_and_alloc_items(self, 4);
}

void NameDict__dtor(NameDict* self) { PK_FREE(self->items); }

py_TValue* NameDict__try_get(NameDict* self, py_Name key) {
    bool ok;
    uintptr_t i;
    HASH_PROBE_0(key, ok, i);
    if(!ok) return NULL;
    return &self->items[i].value;
}

bool NameDict__contains(NameDict* self, py_Name key) {
    bool ok;
    uintptr_t i;
    HASH_PROBE_0(key, ok, i);
    return ok;
}

void NameDict__set(NameDict* self, py_Name key, py_TValue* val) {
    bool ok;
    uintptr_t i;
    HASH_PROBE_1(key, ok, i);
    if(!ok) {
        self->length++;
        if(self->length > self->critical_size) {
            NameDict__rehash_2x(self);
            HASH_PROBE_1(key, ok, i);
        }
        self->items[i].key = key;
    }
    self->items[i].value = *val;
}

bool NameDict__del(NameDict* self, py_Name key) {
    bool ok;
    uintptr_t i;
    HASH_PROBE_0(key, ok, i);
    if(!ok) return false;
    self->items[i].key = NULL;
    self->items[i].value = *py_NIL();
    self->length--;
    /* tidy */
    uintptr_t posToRemove = i;
    uintptr_t posToShift = posToRemove;
    while(true) {
        posToShift = (posToShift + 1) & self->mask;
        if(self->items[posToShift].key == NULL) break;
        uintptr_t hash_z = HASH_KEY(self->items[posToShift].key);
        uintptr_t insertPos = hash_z & self->mask;
        bool cond1 = insertPos <= posToRemove;
        bool cond2 = posToRemove <= posToShift;
        if((cond1 && cond2) ||
           // chain wrapped around capacity
           (posToShift < insertPos && (cond1 || cond2))) {
            NameDict_KV tmp = self->items[posToRemove];
            assert(tmp.key == NULL);
            self->items[posToRemove] = self->items[posToShift];
            self->items[posToShift] = tmp;
            posToRemove = posToShift;
        }
    }
    return true;
}

void NameDict__clear(NameDict* self) {
    for(int i = 0; i < self->capacity; i++) {
        self->items[i].key = NULL;
        self->items[i].value = *py_NIL();
    }
    self->length = 0;
}

#undef HASH_PROBE_0
#undef HASH_PROBE_1
#undef HASH_KEY