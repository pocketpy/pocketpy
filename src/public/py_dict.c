#include "pocketpy/pocketpy.h"

#include "pocketpy/common/utils.h"
#include "pocketpy/objects/object.h"
#include "pocketpy/interpreter/vm.h"

typedef struct {
    py_TValue key;
    py_TValue val;
} DictEntry;

typedef struct {
    int length;
    int capacity;
    int* indices;
    c11_vector /*T=DictEntry*/ entries;
} Dict;

static void Dict__ctor(Dict* self) {
    self->length = 0;
    self->capacity = 16;
    self->indices = malloc(self->capacity * sizeof(int));
    memset(self->indices, -1, self->capacity * sizeof(int));
    c11_vector__ctor(&self->entries, sizeof(DictEntry));
}

static void Dict__dtor(Dict* self) {
    self->length = 0;
    self->capacity = 0;
    free(self->indices);
    c11_vector__dtor(&self->entries);
}

static bool Dict__probe(Dict* self, py_Ref key, DictEntry* out) {
    py_i64 hash;
    if(!py_hash(key, &hash)) return false;
    int mask = self->capacity - 1;
    for(int idx = hash & mask;; idx = (idx + 1) & mask) {
        int idx2 = self->indices[idx];
        DictEntry* slot = c11__at(DictEntry, &self->entries, idx2);
        if(slot){
            int res = py_eq(key, &slot->key);
            if(res == -1) return false;
            return res;
        }else{
            
        }
    }
}

static bool _py_dict__new__(int argc, py_Ref argv) {
    py_newdict(py_retval());
    return true;
}

static bool _py_dict__getitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    py_i64 hash;
    if(!py_hash(py_arg(1), &hash)) return false;
    Dict* self = py_touserdata(argv);

}

static bool _py_dict__setitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(3);
    py_i64 hash;
    if(!py_hash(py_arg(1), &hash)) return false;
}

static bool _py_dict__delitem__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    py_i64 hash;
    if(!py_hash(py_arg(1), &hash)) return false;
}

static bool _py_dict__contains__(int argc, py_Ref argv){
    PY_CHECK_ARGC(2);
    py_i64 hash;
    if(!py_hash(py_arg(1), &hash)) return false;
}

static bool _py_dict__len__(int argc, py_Ref argv){
    PY_CHECK_ARGC(1);
    Dict* self = py_touserdata(argv);
    py_newint(py_retval(), self->length);
    return true;
}

py_Type pk_dict__register() {
    py_Type type = pk_newtype("dict", tp_object, NULL, (void (*)(void*))Dict__dtor, false, false);

    py_bindmagic(type, __new__, _py_dict__new__);
    // py_bindmagic(type, __init__, _py_dict__init__);
    py_bindmagic(type, __getitem__, _py_dict__getitem__);
    py_bindmagic(type, __setitem__, _py_dict__setitem__);
    py_bindmagic(type, __delitem__, _py_dict__delitem__);
    py_bindmagic(type, __contains__, _py_dict__contains__);
    py_bindmagic(type, __len__, _py_dict__len__);
    return type;
}

void py_newdict(py_Ref out) {
    Dict* ud = py_newobject(out, tp_dict, 0, sizeof(Dict));
    Dict__ctor(ud);
}
