#include "pocketpy/interpreter/name.h"
#include "pocketpy/interpreter/vm.h"

void InternedNames__ctor(InternedNames* self) {
    c11_smallmap_s2n__ctor(&self->interned);
    c11_vector__ctor(&self->r_interned, sizeof(RInternedEntry));

    // initialize all magic names
#define MAGIC_METHOD(x)                                                                            \
    if(x != py_name(#x)) abort();
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD
}

void InternedNames__dtor(InternedNames* self) {
    for(int i = 0; i < self->r_interned.length; i++) {
        PK_FREE(c11__getitem(RInternedEntry, &self->r_interned, i).data);
    }
    c11_smallmap_s2n__dtor(&self->interned);
    c11_vector__dtor(&self->r_interned);
}

py_Name py_name(const char* name) {
    c11_sv sv;
    sv.data = name;
    sv.size = strlen(name);
    return py_namev(sv);
}

py_Name py_namev(c11_sv name) {
    InternedNames* self = &pk_current_vm->names;
    uint16_t index = c11_smallmap_s2n__get(&self->interned, name, 0);
    if(index != 0) return index;
    // generate new index
    if(self->interned.length > 65530) c11__abort("py_Name index overflow");
    // NOTE: we must allocate the string in the heap so iterators are not invalidated
    char* p = PK_MALLOC(name.size + 1);
    memcpy(p, name.data, name.size);
    p[name.size] = '\0';
    RInternedEntry* entry = c11_vector__emplace(&self->r_interned);
    entry->data = p;
    entry->size = name.size;
    memset(&entry->obj, 0, sizeof(py_TValue));
    index = self->r_interned.length;  // 1-based
    // save to _interned
    c11_smallmap_s2n__set(&self->interned, (c11_sv){p, name.size}, index);
    assert(self->interned.length == self->r_interned.length);
    return index;
}

const char* py_name2str(py_Name index) {
    InternedNames* self = &pk_current_vm->names;
    assert(index > 0 && index <= self->interned.length);
    return c11__getitem(RInternedEntry, &self->r_interned, index - 1).data;
}

c11_sv py_name2sv(py_Name index) {
    InternedNames* self = &pk_current_vm->names;
    assert(index > 0 && index <= self->interned.length);
    RInternedEntry entry = c11__getitem(RInternedEntry, &self->r_interned, index - 1);
    return (c11_sv){entry.data, entry.size};
}

py_GlobalRef py_name2ref(py_Name index) {
    InternedNames* self = &pk_current_vm->names;
    assert(index > 0 && index <= self->interned.length);
    RInternedEntry* entry = c11__at(RInternedEntry, &self->r_interned, index - 1);
    if(entry->obj.type == tp_nil) {
        c11_sv sv;
        sv.data = entry->data;
        sv.size = entry->size;
        py_newstrv(&entry->obj, sv);
    }
    return &entry->obj;
}
