#include "pocketpy/objects/dict.h"
#include "pocketpy/common/utils.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct pkpy_DictEntry {
    int64_t hash;
    pkpy_Var key;
    pkpy_Var val;
};

inline static int pkpy_Dict__idx_size(const pkpy_Dict* self) {
    if(self->_htcap < 255) return 1;
    if(self->_htcap < 65535) return 2;
    return 4;
}

inline static int pkpy_Dict__idx_null(const pkpy_Dict* self) {
    if(self->_htcap < 255) return 255;
    if(self->_htcap < 65535) return 65535;
    return 4294967295;
}

inline static int pkpy_Dict__ht_byte_size(const pkpy_Dict* self) { return self->_htcap * pkpy_Dict__idx_size(self); }

void pkpy_Dict__ctor(pkpy_Dict* self) {
    self->_version = 0;
    self->count = 0;
    c11_vector__ctor(&self->_entries, sizeof(struct pkpy_DictEntry));
    self->_htcap = 16;
    self->_hashtable = malloc(pkpy_Dict__ht_byte_size(self));
    memset(self->_hashtable, 0xff, pkpy_Dict__ht_byte_size(self));
}

void pkpy_Dict__dtor(pkpy_Dict* self) {
    c11_vector__dtor(&self->_entries);
    free(self->_hashtable);
}

pkpy_Dict pkpy_Dict__copy(const pkpy_Dict* self) {
    int ht_size = pkpy_Dict__ht_byte_size(self);
    void* ht_clone = malloc(ht_size);
    memcpy(ht_clone, self->_hashtable, ht_size);
    return (pkpy_Dict){._version = 0,
                       .count = self->count,
                       ._entries = c11_vector__copy(&self->_entries),
                       ._htcap = self->_htcap,
                       ._hashtable = ht_clone};
}

static int pkpy_Dict__htget(const pkpy_Dict* self, int h) {
    int sz = pkpy_Dict__idx_size(self);
    switch(sz) {
        case 1: return ((uint8_t*)self->_hashtable)[h];
        case 2: return ((uint16_t*)self->_hashtable)[h];
        case 4: return ((uint32_t*)self->_hashtable)[h];
        default: PK_UNREACHABLE();
    }
}

static void pkpy_Dict__htset(pkpy_Dict* self, int h, int v) {
    int sz = pkpy_Dict__idx_size(self);
    switch(sz) {
        case 1: ((uint8_t*)self->_hashtable)[h] = v; break;
        case 2: ((uint16_t*)self->_hashtable)[h] = v; break;
        case 4: ((uint32_t*)self->_hashtable)[h] = v; break;
        default: PK_UNREACHABLE();
    }
}

static int pkpy_Dict__probe0(const pkpy_Dict* self, void* vm, pkpy_Var key, int64_t hash) {
    const int null = pkpy_Dict__idx_null(self);
    const int mask = self->_htcap - 1;
    for(int h = hash & mask;; h = (h + 1) & mask) {
        int idx = pkpy_Dict__htget(self, h);
        if(idx == null) return h;

        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        if(pkpy_Var__is_null(&entry->key)) return h;
    }
    PK_UNREACHABLE();
}

static int pkpy_Dict__probe1(const pkpy_Dict* self, void* vm, pkpy_Var key, int64_t hash) {
    const int null = pkpy_Dict__idx_null(self);
    const int mask = self->_htcap - 1;
    for(int h = hash & mask;; h = (h + 1) & mask) {
        int idx = pkpy_Dict__htget(self, h);
        if(idx == null) return h;

        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        if(pkpy_Var__is_null(&entry->key)) continue;
        if(entry->hash == hash && pkpy_Var__eq__(vm, entry->key, key)) return h;
    }
    PK_UNREACHABLE();
}

static void pkpy_Dict__extendht(pkpy_Dict* self, void* vm) {
    self->_version += 1;
    free(self->_hashtable);
    self->_htcap *= 2;
    self->_hashtable = malloc(pkpy_Dict__ht_byte_size(self));
    memset(self->_hashtable, 0xff, pkpy_Dict__ht_byte_size(self));

    for(int i = 0; i < self->_entries.count; i++) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, i);
        if(pkpy_Var__is_null(&entry->key)) continue;

        int h = pkpy_Dict__probe0(self, vm, entry->key, entry->hash);
        pkpy_Dict__htset(self, h, i);
    }
}

bool pkpy_Dict__set(pkpy_Dict* self, void* vm, pkpy_Var key, pkpy_Var val) {
    int hash = pkpy_Var__hash__(vm, key);
    int h = pkpy_Dict__probe1(self, vm, key, hash);

    int idx = pkpy_Dict__htget(self, h);
    if(idx == pkpy_Dict__idx_null(self)) {
        self->_version += 1;
        idx = self->_entries.count;
        c11_vector__push(struct pkpy_DictEntry,
                         &self->_entries,
                         ((struct pkpy_DictEntry){
                             .hash = hash,
                             .key = key,
                             .val = val,
                         }));
        h = pkpy_Dict__probe0(self, vm, key, hash);
        pkpy_Dict__htset(self, h, idx);
        self->count += 1;
        if(self->count >= self->_htcap * 0.75) pkpy_Dict__extendht(self, vm);
        return true;
    }

    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);

    if(entry->hash == hash || pkpy_Var__eq__(vm, entry->key, key)) {
        entry->val = val;
    } else {
        self->_version += 1;
        self->count += 1;
        h = pkpy_Dict__probe0(self, vm, key, hash);
        idx = pkpy_Dict__htget(self, h);
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        entry->key = key;
        entry->val = val;
        entry->hash = hash;
    }
    return false;
}

bool pkpy_Dict__contains(const pkpy_Dict* self, void* vm, pkpy_Var key) {
    int hash = pkpy_Var__hash__(vm, key);
    int h = pkpy_Dict__probe1(self, vm, key, hash);

    int idx = pkpy_Dict__htget(self, h);
    if(idx == pkpy_Dict__idx_null(self)) return false;

    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
    assert(entry->hash == hash && pkpy_Var__eq__(vm, entry->key, key));
    return true;
}

static bool pkpy_Dict__refactor(pkpy_Dict* self, void* vm) {
    int deleted_slots = self->_entries.count - self->count;
    if(deleted_slots <= 8 || deleted_slots < self->_entries.count * 0.25) return false;

    // shrink
    self->_version += 1;
    free(self->_hashtable);
    while(self->_htcap * 0.375 > self->count && self->_htcap >= 32)
        self->_htcap /= 2;
    self->_hashtable = malloc(pkpy_Dict__ht_byte_size(self));
    memset(self->_hashtable, 0xff, pkpy_Dict__ht_byte_size(self));

    c11_vector old_entries = self->_entries;
    c11_vector__ctor(&self->_entries, sizeof(struct pkpy_DictEntry));
    for(int i = 0; i < old_entries.count; i++) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &old_entries, i);
        if(pkpy_Var__is_null(&entry->key)) continue;

        int j = self->_entries.count;
        c11_vector__push(struct pkpy_DictEntry, &self->_entries, *entry);
        int h = pkpy_Dict__probe0(self, vm, entry->key, entry->hash);
        pkpy_Dict__htset(self, h, j);
    }
    c11_vector__dtor(&old_entries);
    return true;
}

bool pkpy_Dict__del(pkpy_Dict* self, void* vm, pkpy_Var key) {
    int hash = pkpy_Var__hash__(vm, key);
    int h = pkpy_Dict__probe1(self, vm, key, hash);
    int idx = pkpy_Dict__htget(self, h), null = pkpy_Dict__idx_null(self);
    if(idx == null) return false;
    
    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
    assert(entry->hash == hash && pkpy_Var__eq__(vm, entry->key, key));
    self->_version += 1;
    pkpy_Var__set_null(&entry->key);
    self->count -= 1;
    pkpy_Dict__refactor(self, vm);
    return true;
}

const pkpy_Var *pkpy_Dict__try_get(const pkpy_Dict* self, void* vm, pkpy_Var key) {
    int hash = pkpy_Var__hash__(vm, key);
    int h = pkpy_Dict__probe1(self, vm, key, hash);
    
    int idx = pkpy_Dict__htget(self, h);
    if(idx == pkpy_Dict__idx_null(self)) return NULL;
    
    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
    assert(entry->hash == hash && pkpy_Var__eq__(vm, entry->key, key));
    return &entry->val;
}

void pkpy_Dict__update(pkpy_Dict *self, void *vm, const pkpy_Dict *other) {
    for(int i = 0; i < other->_entries.count; i++) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &other->_entries, i);
        if(pkpy_Var__is_null(&entry->key)) continue;
        pkpy_Dict__set(self, vm, entry->key, entry->val);
    }
}

void pkpy_Dict__clear(pkpy_Dict *self) {
    int v = self->_version;
    pkpy_Dict__dtor(self);
    pkpy_Dict__ctor(self);
    self->_version = v + 1;
}

static int pkpy_Dict__next_entry_idx(const pkpy_Dict* self, int idx) {
    if (idx >= self->_entries.count) return idx;
    do {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        if(!pkpy_Var__is_null(&entry->key)) break;
        idx++;
    } while (idx < self->_entries.count);
    return idx;
}

pkpy_DictIter pkpy_Dict__iter(const pkpy_Dict *self) {
    return (pkpy_DictIter){
        ._dict = self,
        ._index = pkpy_Dict__next_entry_idx(self, 0),
        ._version = self->_version,
    };
}

bool pkpy_DictIter__next(pkpy_DictIter *self, pkpy_Var *key, pkpy_Var *val) {
    if(self->_version != self->_dict->_version) return false;
    if(self->_index >= self->_dict->_entries.count) return false;
    
    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_dict->_entries, self->_index);
    assert(!pkpy_Var__is_null(&entry->key));
    if (key) *key = entry->key;
    if (val) *val = entry->val;

    self->_index = pkpy_Dict__next_entry_idx(self->_dict, self->_index + 1);
    return true;
}
