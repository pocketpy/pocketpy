#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/common/sstream.h"
#include "pocketpy/interpreter/types.h"
#include "pocketpy/interpreter/vm.h"

typedef struct {
    Dict* dict;  // weakref for slot 0
    Dict dict_backup;
    DictEntry* curr;
    DictEntry* end;
    int mode;  // 0: keys, 1: values, 2: items
} DictIterator;

#define Dict__step(x) ((x) < mask ? (x) + 1 : 0)

static uint32_t Dict__next_cap(uint32_t cap) {
    switch(cap) {
        case 7: return 17;
        case 17: return 37;
        case 37: return 79;
        case 79: return 163;
        case 163: return 331;
        case 331: return 673;
        case 673: return 1361;
        case 1361: return 2053;
        case 2053: return 3083;
        case 3083: return 4637;
        case 4637: return 6959;
        case 6959: return 10453;
        case 10453: return 15683;
        case 15683: return 23531;
        case 23531: return 35311;
        case 35311: return 52967;
        case 52967: return 79451;
        case 79451: return 119179;
        case 119179: return 178781;
        case 178781: return 268189;
        case 268189: return 402299;
        case 402299: return 603457;
        case 603457: return 905189;
        case 905189: return 1357787;
        case 1357787: return 2036687;
        case 2036687: return 3055043;
        case 3055043: return 4582577;
        case 4582577: return 6873871;
        case 6873871: return 10310819;
        case 10310819: return 15466229;
        case 15466229: return 23199347;
        case 23199347: return 34799021;
        case 34799021: return 52198537;
        case 52198537: return 78297827;
        case 78297827: return 117446801;
        case 117446801: return 176170229;
        case 176170229: return 264255353;
        case 264255353: return 396383041;
        case 396383041: return 594574583;
        case 594574583: return 891861923;
        default: c11__unreachable();
    }
}

static uint64_t Dict__hash_2nd(uint64_t key) {
    // https://gist.github.com/badboy/6267743
    key = (~key) + (key << 21);  // key = (key << 21) - key - 1
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8);  // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4);  // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

static void Dict__ctor(Dict* self, uint32_t capacity, int entries_capacity) {
    self->length = 0;
    self->capacity = capacity;

    size_t indices_size;
    if(self->capacity < UINT16_MAX) {
        self->index_is_short = true;
        indices_size = self->capacity * sizeof(uint16_t);
        self->null_index_value = UINT16_MAX;
    } else {
        self->index_is_short = false;
        indices_size = self->capacity * sizeof(uint32_t);
        self->null_index_value = UINT32_MAX;
    }

    self->indices = PK_MALLOC(indices_size);
    memset(self->indices, -1, indices_size);

    c11_vector__ctor(&self->entries, sizeof(DictEntry));
    c11_vector__reserve(&self->entries, entries_capacity);
}

static void Dict__dtor(Dict* self) {
    self->length = 0;
    self->capacity = 0;
    PK_FREE(self->indices);
    c11_vector__dtor(&self->entries);
}

static uint32_t Dict__get_index(Dict* self, uint32_t index) {
    if(self->index_is_short) {
        uint16_t* indices = self->indices;
        return indices[index];
    } else {
        uint32_t* indices = self->indices;
        return indices[index];
    }
}

static void Dict__swap_null_index(Dict* self, uint32_t pre_z, uint32_t z) {
    if(self->index_is_short) {
        uint16_t* indices = self->indices;
        assert(indices[pre_z] == UINT16_MAX);
        indices[pre_z] = indices[z];
        indices[z] = UINT16_MAX;
    } else {
        uint32_t* indices = self->indices;
        assert(indices[pre_z] == UINT32_MAX);
        indices[pre_z] = indices[z];
        indices[z] = UINT32_MAX;
    }
}

static void Dict__set_index(Dict* self, uint32_t index, uint32_t value) {
    if(self->index_is_short) {
        uint16_t* indices = self->indices;
        indices[index] = (uint16_t)value;
    } else {
        uint32_t* indices = self->indices;
        indices[index] = value;
    }
}

// Dict__probe won't raise exception for string keys
static bool Dict__probe(Dict* self,
                        py_TValue* key,
                        uint64_t* p_hash,
                        uint32_t* p_idx,
                        DictEntry** p_entry) {
    if(py_isstr(key)) {
        *p_hash = c11_sv__hash(py_tosv(key));
    } else {
        py_i64 h_user;
        if(!py_hash(key, &h_user)) return false;
        *p_hash = Dict__hash_2nd((uint64_t)h_user);
    }
    uint32_t mask = self->capacity - 1;
    uint32_t idx = (*p_hash) % self->capacity;
    while(true) {
        uint32_t idx2 = Dict__get_index(self, idx);
        if(idx2 == self->null_index_value) break;
        DictEntry* entry = c11__at(DictEntry, &self->entries, idx2);
        if(entry->hash == (*p_hash)) {
            if(py_isstr(&entry->key) && py_isstr(key)) {
                c11_sv lhs = py_tosv(&entry->key);
                c11_sv rhs = py_tosv(key);
                if(c11__sveq(lhs, rhs)) {
                    *p_idx = idx;
                    *p_entry = entry;
                    return true;
                }
            } else {
                int res = py_equal(&entry->key, key);
                if(res == 1) {
                    *p_idx = idx;
                    *p_entry = entry;
                    return true;
                }
                if(res == -1) return false;  // error
            }
        }
        // try next index
        idx = Dict__step(idx);
    }
    // not found
    *p_idx = idx;
    *p_entry = NULL;
    return true;
}

static bool Dict__try_get(Dict* self, py_TValue* key, DictEntry** out) {
    uint64_t hash;
    uint32_t idx;
    return Dict__probe(self, key, &hash, &idx, out);
}

static void Dict__clear(Dict* self) {
    size_t indices_size = self->index_is_short ? self->capacity * sizeof(uint16_t)
                                               : self->capacity * sizeof(uint32_t);
    memset(self->indices, -1, indices_size);
    c11_vector__clear(&self->entries);
    self->length = 0;
}

static void Dict__rehash_2x(Dict* self) {
    Dict old_dict = *self;
    uint32_t new_capacity = Dict__next_cap(old_dict.capacity);
    uint32_t mask = new_capacity - 1;
    // create a new dict with new capacity
    Dict__ctor(self, new_capacity, old_dict.entries.capacity);
    // move entries from old dict to new dict
    for(int i = 0; i < old_dict.entries.length; i++) {
        DictEntry* old_entry = c11__at(DictEntry, &old_dict.entries, i);
        if(py_isnil(&old_entry->key)) continue;  // skip deleted
        uint32_t idx = old_entry->hash % new_capacity;
        while(true) {
            uint32_t idx2 = Dict__get_index(self, idx);
            if(idx2 == self->null_index_value) {
                c11_vector__push(DictEntry, &self->entries, *old_entry);
                Dict__set_index(self, idx, self->entries.length - 1);
                self->length++;
                break;
            }
            // try next index
            idx = Dict__step(idx);
        }
    }
    Dict__dtor(&old_dict);
}

static void Dict__compact_entries(Dict* self) {
    uint32_t* mappings = PK_MALLOC(self->entries.length * sizeof(uint32_t));

    int n = 0;
    for(int i = 0; i < self->entries.length; i++) {
        DictEntry* entry = c11__at(DictEntry, &self->entries, i);
        if(py_isnil(&entry->key)) continue;
        mappings[i] = n;
        if(i != n) {
            DictEntry* new_entry = c11__at(DictEntry, &self->entries, n);
            *new_entry = *entry;
        }
        n++;
    }
    self->entries.length = n;
    // update indices
    for(uint32_t idx = 0; idx < self->capacity; idx++) {
        uint32_t idx2 = Dict__get_index(self, idx);
        if(idx2 == self->null_index_value) continue;
        Dict__set_index(self, idx, mappings[idx2]);
    }
    PK_FREE(mappings);
}

static bool Dict__set(Dict* self, py_TValue* key, py_TValue* val) {
    uint64_t hash;
    uint32_t idx;
    DictEntry* entry;
    if(!Dict__probe(self, key, &hash, &idx, &entry)) return false;
    if(entry) {
        // update existing entry
        entry->val = *val;
        return true;
    }
    // insert new entry
    DictEntry* new_entry = c11_vector__emplace(&self->entries);
    new_entry->hash = hash;
    new_entry->key = *key;
    new_entry->val = *val;
    Dict__set_index(self, idx, self->entries.length - 1);
    self->length++;
    // check if we need to rehash
    float load_factor = (float)self->length / self->capacity;
    if(load_factor > (self->index_is_short ? 0.3f : 0.4f)) Dict__rehash_2x(self);
    return true;
}

/// Delete an entry from the dict.
/// -1: error, 0: not found, 1: found and deleted
static int Dict__pop(Dict* self, py_Ref key) {
    // Dict__log_index(self, "before pop");
    uint64_t hash;
    uint32_t idx;
    DictEntry* entry;
    if(!Dict__probe(self, key, &hash, &idx, &entry)) return -1;
    if(!entry) return 0;  // not found

    // found the entry, delete and return it
    py_assign(py_retval(), &entry->val);
    Dict__set_index(self, idx, self->null_index_value);
    py_newnil(&entry->key);
    py_newnil(&entry->val);
    self->length--;

    /* tidy */
    // https://github.com/OpenHFT/Chronicle-Map/blob/820573a68471509ffc1b0584454f4a67c0be1b84/src/main/java/net/openhft/chronicle/hash/impl/CompactOffHeapLinearHashTable.java#L156
    uint32_t mask = self->capacity - 1;
    uint32_t posToRemove = idx;
    uint32_t posToShift = posToRemove;
    // int probe_count = 0;
    // int swap_count = 0;
    while(true) {
        posToShift = Dict__step(posToShift);
        uint32_t idx_z = Dict__get_index(self, posToShift);
        if(idx_z == self->null_index_value) break;
        uint64_t hash_z = c11__at(DictEntry, &self->entries, idx_z)->hash;
        uint32_t insertPos = (uint64_t)hash_z % self->capacity;
        // the following condition essentially means circular permutations
        // of three (r = posToRemove, s = posToShift, i = insertPos)
        // positions are accepted:
        // [...i..r...s.] or
        // [...r..s...i.] or
        // [...s..i...r.]
        bool cond1 = insertPos <= posToRemove;
        bool cond2 = posToRemove <= posToShift;
        if((cond1 && cond2) ||
           // chain wrapped around capacity
           (posToShift < insertPos && (cond1 || cond2))) {
            Dict__swap_null_index(self, posToRemove, posToShift);
            posToRemove = posToShift;
            // swap_count++;
        }
        // probe_count++;
    }
    // printf("Dict__pop: probe_count=%d, swap_count=%d\n", probe_count, swap_count);
    // compact entries if necessary
    if(self->entries.length > 16 && (self->length < self->entries.length >> 1)) {
        Dict__compact_entries(self);  // compact entries
    }
    // Dict__log_index(self, "after pop");
    return 1;
}

static void DictIterator__ctor(DictIterator* self, Dict* dict, int mode) {
    assert(mode >= 0 && mode <= 2);
    self->dict = dict;
    self->dict_backup = *dict;  // backup the dict
    self->curr = dict->entries.data;
    self->end = self->curr + dict->entries.length;
    self->mode = mode;
}

static DictEntry* DictIterator__next(DictIterator* self) {
    DictEntry* retval;
    do {
        if(self->curr == self->end) return NULL;
        retval = self->curr++;
    } while(py_isnil(&retval->key));
    return retval;
}

static bool DictIterator__modified(DictIterator* self) {
    return memcmp(self->dict, &self->dict_backup, sizeof(Dict)) != 0;
}

///////////////////////////////
static bool dict__new__(int argc, py_Ref argv) {
    py_Type cls = py_totype(argv);
    int slots = cls == tp_dict ? 0 : -1;
    Dict* ud = py_newobject(py_retval(), cls, slots, sizeof(Dict));
    Dict__ctor(ud, 17, 4);
    return true;
}

void py_newdict(py_OutRef out) {
    Dict* ud = py_newobject(out, tp_dict, 0, sizeof(Dict));
    Dict__ctor(ud, 17, 4);
}

static bool dict__init__(int argc, py_Ref argv) {
    if(argc > 2) return TypeError("dict.__init__() takes at most 2 arguments (%d given)", argc);
    if(argc == 1) {
        py_newnone(py_retval());
        return true;
    }
    assert(argc == 2);
    py_TValue* p;
    int length = pk_arrayview(py_arg(1), &p);
    if(length == -1) { return TypeError("dict.__init__() expects a list or tuple"); }

    Dict* self = py_touserdata(argv);
    for(int i = 0; i < length; i++) {
        py_Ref tuple = &p[i];
        if(!py_istuple(tuple) || py_tuple_len(tuple) != 2) {
            return ValueError("dict.__init__() argument must be a list of tuple-2");
        }
        py_Ref key = py_tuple_getitem(tuple, 0);
        py_Ref val = py_tuple_getitem(tuple, 1);
        if(!Dict__set(self, key, val)) return false;
    }
    py_newnone(py_retval());
    return true;
}

static bool dict__getitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    Dict* self = py_touserdata(argv);
    DictEntry* entry;
    if(!Dict__try_get(self, py_arg(1), &entry)) return false;
    if(entry) {
        *py_retval() = entry->val;
        return true;
    }
    // try __missing__
    py_Ref missing = py_tpfindmagic(argv->type, __missing__);
    if(missing) return py_call(missing, argc, argv);
    return KeyError(py_arg(1));
}

static bool dict__setitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    Dict* self = py_touserdata(argv);
    bool ok = Dict__set(self, py_arg(1), py_arg(2));
    if(ok) py_newnone(py_retval());
    return ok;
}

static bool dict__delitem__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    Dict* self = py_touserdata(argv);
    int res = Dict__pop(self, py_arg(1));
    if(res == 1) {
        py_newnone(py_retval());
        return true;
    }
    if(res == 0) return KeyError(py_arg(1));
    return false;
}

static bool dict__contains__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    Dict* self = py_touserdata(argv);
    DictEntry* entry;
    if(!Dict__try_get(self, py_arg(1), &entry)) return false;
    py_newbool(py_retval(), entry != NULL);
    return true;
}

static bool dict__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    py_newint(py_retval(), self->length);
    return true;
}

static bool dict__repr__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    c11_sbuf__write_char(&buf, '{');
    bool is_first = true;
    for(int i = 0; i < self->entries.length; i++) {
        DictEntry* entry = c11__at(DictEntry, &self->entries, i);
        if(py_isnil(&entry->key)) continue;
        if(!is_first) c11_sbuf__write_cstr(&buf, ", ");
        if(!py_repr(&entry->key)) return false;
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
        c11_sbuf__write_cstr(&buf, ": ");
        if(!py_repr(&entry->val)) return false;
        c11_sbuf__write_sv(&buf, py_tosv(py_retval()));
        is_first = false;
    }
    c11_sbuf__write_char(&buf, '}');
    c11_sbuf__py_submit(&buf, py_retval());
    return true;
}

static bool dict__eq__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    Dict* self = py_touserdata(py_arg(0));
    if(!py_isdict(py_arg(1))) {
        py_newnotimplemented(py_retval());
        return true;
    }
    Dict* other = py_touserdata(py_arg(1));
    if(self->length != other->length) {
        py_newbool(py_retval(), false);
        return true;
    }
    DictIterator iter;
    DictIterator__ctor(&iter, self, 2);
    // for each self key
    while(1) {
        DictEntry* entry = DictIterator__next(&iter);
        if(!entry) break;
        DictEntry* other_entry;
        if(!Dict__try_get(other, &entry->key, &other_entry)) return false;
        if(!other_entry) {
            py_newbool(py_retval(), false);
            return true;
        }
        if(entry->hash != other_entry->hash) {
            py_newbool(py_retval(), false);
            return true;
        }
        int res = py_equal(&entry->val, &other_entry->val);
        if(res == 0) {
            py_newbool(py_retval(), false);
            return true;
        }
        if(res == -1) return false;  // error
    }
    py_newbool(py_retval(), true);
    return true;
}

static bool dict__ne__(int argc, py_Ref argv) {
    if(!dict__eq__(argc, argv)) return false;
    if(py_isbool(py_retval())) {
        bool res = py_tobool(py_retval());
        py_newbool(py_retval(), !res);
    }
    return true;
}

static bool dict_clear(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    Dict__clear(self);
    py_newnone(py_retval());
    return true;
}

static bool dict_copy(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    Dict* new_dict = py_newobject(py_retval(), tp_dict, 0, sizeof(Dict));
    new_dict->length = self->length;
    new_dict->capacity = self->capacity;
    new_dict->null_index_value = self->null_index_value;
    new_dict->index_is_short = self->index_is_short;
    // copy entries
    new_dict->entries = c11_vector__copy(&self->entries);
    // copy indices
    size_t indices_size = self->index_is_short ? self->capacity * sizeof(uint16_t)
                                               : self->capacity * sizeof(uint32_t);
    new_dict->indices = PK_MALLOC(indices_size);
    memcpy(new_dict->indices, self->indices, indices_size);
    return true;
}

static bool dict_update(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_dict);
    Dict* self = py_touserdata(argv);
    Dict* other = py_touserdata(py_arg(1));
    for(int i = 0; i < other->entries.length; i++) {
        DictEntry* entry = c11__at(DictEntry, &other->entries, i);
        if(py_isnil(&entry->key)) continue;
        if(!Dict__set(self, &entry->key, &entry->val)) return false;
    }
    py_newnone(py_retval());
    return true;
}

static bool dict_get(int argc, py_Ref argv) {
    Dict* self = py_touserdata(argv);
    if(argc > 3) return TypeError("get() takes at most 3 arguments (%d given)", argc);
    py_Ref default_val = argc == 3 ? py_arg(2) : py_None();
    DictEntry* entry;
    if(!Dict__try_get(self, py_arg(1), &entry)) return false;
    *py_retval() = entry ? entry->val : *default_val;
    return true;
}

static bool dict_pop(int argc, py_Ref argv) {
    Dict* self = py_touserdata(argv);
    if(argc < 2 || argc > 3) return TypeError("pop() takes 1 or 2 arguments (%d given)", argc - 1);
    py_Ref default_val = argc == 3 ? py_arg(2) : py_None();
    int res = Dict__pop(self, py_arg(1));
    if(res == -1) return false;
    if(res == 0) py_assign(py_retval(), default_val);
    return true;
}

static bool dict_keys(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    DictIterator* ud = py_newobject(py_retval(), tp_dict_iterator, 1, sizeof(DictIterator));
    DictIterator__ctor(ud, self, 0);
    py_setslot(py_retval(), 0, argv);  // keep a reference to the dict
    return true;
}

static bool dict_values(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    DictIterator* ud = py_newobject(py_retval(), tp_dict_iterator, 1, sizeof(DictIterator));
    DictIterator__ctor(ud, self, 1);
    py_setslot(py_retval(), 0, argv);  // keep a reference to the dict
    return true;
}

static bool dict_items(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    DictIterator* ud = py_newobject(py_retval(), tp_dict_iterator, 1, sizeof(DictIterator));
    DictIterator__ctor(ud, self, 2);
    py_setslot(py_retval(), 0, argv);  // keep a reference to the dict
    return true;
}

py_Type pk_dict__register() {
    py_Type type = pk_newtype("dict", tp_object, NULL, (void (*)(void*))Dict__dtor, false, false);

    py_bindmagic(type, __new__, dict__new__);
    py_bindmagic(type, __init__, dict__init__);
    py_bindmagic(type, __getitem__, dict__getitem__);
    py_bindmagic(type, __setitem__, dict__setitem__);
    py_bindmagic(type, __delitem__, dict__delitem__);
    py_bindmagic(type, __contains__, dict__contains__);
    py_bindmagic(type, __len__, dict__len__);
    py_bindmagic(type, __repr__, dict__repr__);
    py_bindmagic(type, __eq__, dict__eq__);
    py_bindmagic(type, __ne__, dict__ne__);
    py_bindmagic(type, __iter__, dict_keys);

    py_bindmethod(type, "clear", dict_clear);
    py_bindmethod(type, "copy", dict_copy);
    py_bindmethod(type, "update", dict_update);
    py_bindmethod(type, "get", dict_get);
    py_bindmethod(type, "pop", dict_pop);
    py_bindmethod(type, "keys", dict_keys);
    py_bindmethod(type, "values", dict_values);
    py_bindmethod(type, "items", dict_items);

    py_setdict(py_tpobject(type), __hash__, py_None());
    return type;
}

//////////////////////////
bool dict_items__next__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    DictIterator* iter = py_touserdata(py_arg(0));
    if(DictIterator__modified(iter)) return RuntimeError("dictionary modified during iteration");
    DictEntry* entry = (DictIterator__next(iter));
    if(entry) {
        switch(iter->mode) {
            case 0:  // keys
                py_assign(py_retval(), &entry->key);
                return true;
            case 1:  // values
                py_assign(py_retval(), &entry->val);
                return true;
            case 2:  // items
            {
                py_Ref p = py_newtuple(py_retval(), 2);
                p[0] = entry->key;
                p[1] = entry->val;
                return true;
            }
            default: c11__unreachable();
        }
    }
    return StopIteration();
}

bool dict_items__len__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    DictIterator* iter = py_touserdata(py_arg(0));
    py_newint(py_retval(), iter->dict->length);
    return true;
}

py_Type pk_dict_items__register() {
    py_Type type = pk_newtype("dict_iterator", tp_object, NULL, NULL, false, true);
    py_bindmagic(type, __iter__, pk_wrapper__self);
    py_bindmagic(type, __next__, dict_items__next__);
    py_bindmagic(type, __len__, dict_items__len__);
    return type;
}

//////////////////////////

int py_dict_getitem(py_Ref self, py_Ref key) {
    assert(py_isdict(self));
    Dict* ud = py_touserdata(self);
    DictEntry* entry;
    if(!Dict__try_get(ud, key, &entry)) return -1;
    if(entry) {
        py_assign(py_retval(), &entry->val);
        return 1;
    }
    return 0;
}

bool py_dict_setitem(py_Ref self, py_Ref key, py_Ref val) {
    assert(py_isdict(self));
    Dict* ud = py_touserdata(self);
    return Dict__set(ud, key, val);
}

int py_dict_delitem(py_Ref self, py_Ref key) {
    assert(py_isdict(self));
    Dict* ud = py_touserdata(self);
    return Dict__pop(ud, key);
}

int py_dict_getitem_by_str(py_Ref self, const char* key) {
    py_Ref tmp = py_pushtmp();
    py_newstr(tmp, key);
    int res = py_dict_getitem(self, tmp);
    py_pop();
    return res;
}

bool py_dict_setitem_by_str(py_Ref self, const char* key, py_Ref val) {
    py_Ref tmp = py_pushtmp();
    py_newstr(tmp, key);
    bool res = py_dict_setitem(self, tmp, val);
    py_pop();
    return res;
}

int py_dict_delitem_by_str(py_Ref self, const char* key) {
    py_Ref tmp = py_pushtmp();
    py_newstr(tmp, key);
    int res = py_dict_delitem(self, tmp);
    py_pop();
    return res;
}

int py_dict_getitem_by_int(py_Ref self, py_i64 key) {
    py_TValue tmp;
    py_newint(&tmp, key);
    return py_dict_getitem(self, &tmp);
}

bool py_dict_setitem_by_int(py_Ref self, py_i64 key, py_Ref val) {
    py_TValue tmp;
    py_newint(&tmp, key);
    return py_dict_setitem(self, &tmp, val);
}

int py_dict_delitem_by_int(py_Ref self, py_i64 key) {
    py_TValue tmp;
    py_newint(&tmp, key);
    return py_dict_delitem(self, &tmp);
}

int py_dict_len(py_Ref self) {
    assert(py_isdict(self));
    Dict* ud = py_touserdata(self);
    return ud->length;
}

bool py_dict_apply(py_Ref self, bool (*f)(py_Ref, py_Ref, void*), void* ctx) {
    Dict* ud = py_touserdata(self);
    for(int i = 0; i < ud->entries.length; i++) {
        DictEntry* entry = c11__at(DictEntry, &ud->entries, i);
        if(py_isnil(&entry->key)) continue;
        if(!f(&entry->key, &entry->val, ctx)) return false;
    }
    return true;
}

#undef Dict__step