#include "pocketpy/objects/dict.h"
#include "pocketpy/common/utils.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define DICT_MAX_LOAD 0.75
#define DICT_HASH_NEXT(h) ((h) * 5 + 1)
#define DICT_HASH_TRANS(h) ((int)((h) & 0xffffffff)) // used for tansform value from __hash__
#define PK_DICT_COMPACT_MODE 1

struct pkpy_DictEntry {
    pkpy_Var key;
    pkpy_Var val;
};

inline extern int pkpy_Dict__idx_size(const pkpy_Dict* self) {
#if PK_DICT_COMPACT_MODE
    if(self->_htcap < 255) return 1;
    if(self->_htcap < 65535) return 2;
#endif
    return 4;
}

inline extern int pkpy_Dict__idx_null(const pkpy_Dict* self) {
#if PK_DICT_COMPACT_MODE
    if(self->_htcap < 255) return 255;
    if(self->_htcap < 65535) return 65535;
#endif
    return -1;
}

inline extern int pkpy_Dict__ht_byte_size(const pkpy_Dict* self) { return self->_htcap * pkpy_Dict__idx_size(self); }

void pkpy_Dict__ctor(pkpy_Dict* self) {
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
    return (pkpy_Dict){.count = self->count,
                       ._entries = c11_vector__copy(&self->_entries),
                       ._htcap = self->_htcap,
                       ._hashtable = ht_clone};
}

static int pkpy_Dict__htget(const pkpy_Dict* self, int h) {
#if PK_DICT_COMPACT_MODE
    const int *p = (const int*)(((const char*)self->_hashtable) + h * pkpy_Dict__idx_size(self));
    return (*p) & pkpy_Dict__idx_null(self);
#else
    return ((const int*)self->_hashtable)[h];
#endif
}

static void pkpy_Dict__htset(pkpy_Dict* self, int h, int v) {
#if PK_DICT_COMPACT_MODE
    int *p = (int*)(((char*)self->_hashtable) + h * pkpy_Dict__idx_size(self));
    *p = v | (*p & ~pkpy_Dict__idx_null(self));
#else
    ((int*)self->_hashtable)[h] = v;
#endif
}

static int pkpy_Dict__probe0(const pkpy_Dict* self, void* vm, pkpy_Var key, int hash) {
    const int null = pkpy_Dict__idx_null(self);
    const int mask = self->_htcap - 1;
    for(int h = hash & mask;; h = DICT_HASH_NEXT(h) & mask) {
        int idx = pkpy_Dict__htget(self, h);
        if(idx == null) return h;

        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        if(pkpy_Var__is_null(&entry->key)) return h;
    }
    PK_UNREACHABLE();
}

static int pkpy_Dict__probe1(const pkpy_Dict* self, void* vm, pkpy_Var key, int hash) {
    const int null = pkpy_Dict__idx_null(self);
    const int mask = self->_htcap - 1;
    for(int h = hash & mask;; h = DICT_HASH_NEXT(h) & mask) {
        int idx = pkpy_Dict__htget(self, h);
        if(idx == null) return h;

        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        if(pkpy_Var__is_null(&entry->key)) continue;
        if(pkpy_Var__eq__(vm, entry->key, key)) return h;
    }
    PK_UNREACHABLE();
}

static void pkpy_Dict__extendht(pkpy_Dict* self, void* vm) {
    free(self->_hashtable);
    self->_htcap *= 2;
    self->_hashtable = malloc(pkpy_Dict__ht_byte_size(self));
    memset(self->_hashtable, 0xff, pkpy_Dict__ht_byte_size(self));

    for(int i = 0; i < self->_entries.count; i++) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, i);
        if(pkpy_Var__is_null(&entry->key)) continue;

        int rhash = DICT_HASH_TRANS(pkpy_Var__hash__(vm, entry->key));
        int h = pkpy_Dict__probe0(self, vm, entry->key, rhash);
        pkpy_Dict__htset(self, h, i);
    }
}

bool pkpy_Dict__set(pkpy_Dict* self, void* vm, pkpy_Var key, pkpy_Var val) {
    int hash = DICT_HASH_TRANS(pkpy_Var__hash__(vm, key));
    int h = pkpy_Dict__probe1(self, vm, key, hash);

    int idx = pkpy_Dict__htget(self, h);
    if(idx == pkpy_Dict__idx_null(self)) {
        idx = self->_entries.count;
        c11_vector__push(struct pkpy_DictEntry,
                         &self->_entries,
                         ((struct pkpy_DictEntry){
                             .key = key,
                             .val = val,
                         }));
        h = pkpy_Dict__probe0(self, vm, key, hash);
        pkpy_Dict__htset(self, h, idx);
        self->count += 1;
        if(self->count >= self->_htcap * DICT_MAX_LOAD) pkpy_Dict__extendht(self, vm);
        return true;
    }

    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);

    if(pkpy_Var__eq__(vm, entry->key, key)) {
        entry->val = val;
    } else {
        self->count += 1;
        h = pkpy_Dict__probe0(self, vm, key, hash);
        idx = pkpy_Dict__htget(self, h);
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        entry->key = key;
        entry->val = val;
    }
    return false;
}

bool pkpy_Dict__contains(const pkpy_Dict* self, void* vm, pkpy_Var key) {
    int hash = DICT_HASH_TRANS(pkpy_Var__hash__(vm, key));
    int h = pkpy_Dict__probe1(self, vm, key, hash);

    int idx = pkpy_Dict__htget(self, h);
    if(idx == pkpy_Dict__idx_null(self)) return false;
    return true;
}

static bool pkpy_Dict__refactor(pkpy_Dict* self, void* vm) {
    int deleted_slots = self->_entries.count - self->count;
    if(deleted_slots <= 8 || deleted_slots < self->_entries.count * (1 - DICT_MAX_LOAD)) return false;

    // shrink
    // free(self->_hashtable);
    // while(self->_htcap * DICT_MAX_LOAD / 2 > self->count && self->_htcap >= 32)
    //     self->_htcap /= 2;
    // self->_hashtable = malloc(pkpy_Dict__ht_byte_size(self));
    memset(self->_hashtable, 0xff, pkpy_Dict__ht_byte_size(self));

    int new_cnt = 0;
    for (int i = 0; i < self->_entries.count; ++i) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, i);
        if(pkpy_Var__is_null(&entry->key)) continue;
        if (i > new_cnt) c11__setitem(struct pkpy_DictEntry, &self->_entries, new_cnt, *entry);
        new_cnt += 1;
    }

    self->_entries.count = new_cnt;
    for(int i = 0; i < self->_entries.count; i++) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, i);
        if(pkpy_Var__is_null(&entry->key)) continue;

        int rhash = DICT_HASH_TRANS(pkpy_Var__hash__(vm, entry->key));
        int h = pkpy_Dict__probe0(self, vm, entry->key, rhash);
        pkpy_Dict__htset(self, h, i);
    }
    return true;
}

bool pkpy_Dict__del(pkpy_Dict* self, void* vm, pkpy_Var key) {
    int hash = DICT_HASH_TRANS(pkpy_Var__hash__(vm, key));
    int h = pkpy_Dict__probe1(self, vm, key, hash);
    int idx = pkpy_Dict__htget(self, h), null = pkpy_Dict__idx_null(self);
    if(idx == null) return false;
    
    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
    pkpy_Var__set_null(&entry->key);
    self->count -= 1;
    pkpy_Dict__refactor(self, vm);
    return true;
}

const pkpy_Var *pkpy_Dict__try_get(const pkpy_Dict* self, void* vm, pkpy_Var key) {
    int hash = DICT_HASH_TRANS(pkpy_Var__hash__(vm, key));
    int h = pkpy_Dict__probe1(self, vm, key, hash);
    
    int idx = pkpy_Dict__htget(self, h);
    if(idx == pkpy_Dict__idx_null(self)) return NULL;
    
    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
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
    self->count = 0;
    self->_entries.count = 0;
    memset(self->_hashtable, 0xff, pkpy_Dict__ht_byte_size(self));
}

static int pkpy_Dict__next_entry_idx(const pkpy_Dict* self, int idx) {
    while (idx < self->_entries.count) {
        struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_entries, idx);
        if(!pkpy_Var__is_null(&entry->key)) break;
        idx++;
    }
    return idx;
}

pkpy_DictIter pkpy_Dict__iter(const pkpy_Dict *self) {
    return (pkpy_DictIter){
        ._dict = self,
        ._index = pkpy_Dict__next_entry_idx(self, 0),
    };
}

bool pkpy_DictIter__next(pkpy_DictIter *self, pkpy_Var *key, pkpy_Var *val) {
    if(self->_index >= self->_dict->_entries.count) return false;
    
    struct pkpy_DictEntry* entry = &c11__getitem(struct pkpy_DictEntry, &self->_dict->_entries, self->_index);
    if(pkpy_Var__is_null(&entry->key)) return false;
    if (key) *key = entry->key;
    if (val) *val = entry->val;

    self->_index = pkpy_Dict__next_entry_idx(self->_dict, self->_index + 1);
    return true;
}
