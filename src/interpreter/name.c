#include "pocketpy/interpreter/name.h"
#include "pocketpy/interpreter/vm.h"

#define MAGIC_METHOD(x) py_Name x;
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD

void InternedNames__ctor(InternedNames* self) {
    c11_smallmap_s2n__ctor(&self->interned);

    // initialize all magic names
#define MAGIC_METHOD(x) x = py_name(#x);
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD
}

void InternedNames__dtor(InternedNames* self) {
    for(int i = 0; i < self->interned.length; i++) {
        c11_smallmap_s2n_KV* kv = c11__at(c11_smallmap_s2n_KV, &self->interned, i);
        PK_FREE((void*)kv->value);
    }
    c11_smallmap_s2n__dtor(&self->interned);
}

py_Name py_name(const char* name) {
    c11_sv sv;
    sv.data = name;
    sv.size = strlen(name);
    return py_namev(sv);
}

py_Name py_namev(c11_sv name) {
    InternedNames* self = &pk_current_vm->names;
    py_Name index = c11_smallmap_s2n__get(&self->interned, name, 0);
    if(index != 0) return index;
    // generate new index
    InternedEntry* p = PK_MALLOC(sizeof(InternedEntry) + name.size + 1);
    p->size = name.size;
    memcpy(p->data, name.data, name.size);
    p->data[name.size] = '\0';
    memset(&p->obj, 0, sizeof(py_TValue));
    index = (py_Name)p;
    // save to _interned
    c11_smallmap_s2n__set(&self->interned, (c11_sv){p->data, name.size}, index);
    return index;
}

const char* py_name2str(py_Name index) {
    InternedEntry* p = (InternedEntry*)index;
    return p->data;
}

c11_sv py_name2sv(py_Name index) {
    InternedEntry* p = (InternedEntry*)index;
    return (c11_sv){p->data, p->size};
}

py_GlobalRef py_name2ref(py_Name index) {
    InternedEntry* p = (InternedEntry*)index;
    if(p->obj.type == tp_nil) {
        c11_sv sv;
        sv.data = p->data;
        sv.size = p->size;
        py_newstrv(&p->obj, sv);
    }
    return &p->obj;
}
