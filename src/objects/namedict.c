#include "pocketpy/objects/namedict.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/pocketpy.h"
#include <stdint.h>

void ModuleDict__ctor(ModuleDict* self, const char* path, py_TValue module) {
    assert(path != NULL);
    int length = strlen(path);
    assert(length <= PK_MAX_MODULE_PATH_LEN);
    memcpy(self->path, path, length);
    self->path[length] = '\0';
    self->module = module;
    self->left = NULL;
    self->right = NULL;
}

void ModuleDict__dtor(ModuleDict* self) {
    if(self->left) {
        ModuleDict__dtor(self->left);
        PK_FREE(self->left);
    }
    if(self->right) {
        ModuleDict__dtor(self->right);
        PK_FREE(self->right);
    }
}

void ModuleDict__set(ModuleDict* self, const char* key, py_TValue val) {
    assert(key != NULL);
    int cmp = strcmp(key, self->path);
    if(cmp < 0) {
        if(self->left) {
            ModuleDict__set(self->left, key, val);
        } else {
            self->left = PK_MALLOC(sizeof(ModuleDict));
            ModuleDict__ctor(self->left, key, val);
        }
    } else if(cmp > 0) {
        if(self->right) {
            ModuleDict__set(self->right, key, val);
        } else {
            self->right = PK_MALLOC(sizeof(ModuleDict));
            ModuleDict__ctor(self->right, key, val);
        }
    } else {
        self->module = val;
    }
}

py_TValue* ModuleDict__try_get(ModuleDict* self, const char* path) {
    assert(path != NULL);
    int cmp = strcmp(path, self->path);
    if(cmp < 0) {
        if(self->left) {
            return ModuleDict__try_get(self->left, path);
        } else {
            return NULL;
        }
    } else if(cmp > 0) {
        if(self->right) {
            return ModuleDict__try_get(self->right, path);
        } else {
            return NULL;
        }
    } else {
        return &self->module;
    }
}

bool ModuleDict__contains(ModuleDict* self, const char* path) {
    assert(path != NULL);
    return ModuleDict__try_get(self, path) != NULL;
}

void ModuleDict__apply_mark(ModuleDict* self, c11_vector* p_stack) {
    if(!py_isnil(&self->module)) {
        // root node is dummy
        PyObject* obj = self->module._obj;
        assert(obj != NULL);
        if(!obj->gc_marked) {
            obj->gc_marked = true;
            c11_vector__push(PyObject*, p_stack, obj);
        }
    }
    if(self->left) ModuleDict__apply_mark(self->left, p_stack);
    if(self->right) ModuleDict__apply_mark(self->right, p_stack);
}

/////////////////// NameDict ///////////////////

#define HASH_PROBE_1(__k, ok, i)                                                                   \
    ok = false;                                                                                    \
    i = (uintptr_t)(__k) & self->mask;                                                             \
    while(self->items[i].key != NULL) {                                                            \
        if(self->items[i].key == (__k)) {                                                          \
            ok = true;                                                                             \
            break;                                                                                 \
        }                                                                                          \
        i = (i + 1) & self->mask;                                                                  \
    }

#define HASH_PROBE_0 HASH_PROBE_1

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
    // tidy
    uintptr_t pre_z = i;
    uintptr_t z = (i + 1) & self->mask;
    while(self->items[z].key != NULL) {
        uintptr_t h = (uintptr_t)self->items[z].key & self->mask;
        if(h != i) break;
        // std::swap(_items[pre_z], _items[z]);
        NameDict_KV tmp = self->items[pre_z];
        self->items[pre_z] = self->items[z];
        self->items[z] = tmp;
        pre_z = z;
        z = (z + 1) & self->mask;
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