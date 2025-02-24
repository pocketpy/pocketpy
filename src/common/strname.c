#include "pocketpy/common/strname.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#include <stdio.h>

typedef struct {
    char* data;     // null-terminated data
    int size;       // size of the data excluding the null-terminator
    py_TValue* ref; // cached `str` object (lazy initialized)
} RInternedEntry;

// TODO: use a more efficient data structure
static c11_smallmap_s2n _interned;
static c11_vector /* T=RInternedEntry */ _r_interned;

void py_Name__initialize() {
    c11_smallmap_s2n__ctor(&_interned);
    c11_vector__ctor(&_r_interned, sizeof(RInternedEntry));

#define MAGIC_METHOD(x)                                                                            \
    if(x != py_name(#x)) abort();
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD
}

void py_Name__finalize() {
    // free all char*
    for(int i = 0; i < _r_interned.length; i++) {
        PK_FREE(c11__getitem(RInternedEntry, &_r_interned, i).data);
    }
    c11_smallmap_s2n__dtor(&_interned);
    c11_vector__dtor(&_r_interned);
}

py_Name py_name(const char* name) { return py_namev((c11_sv){name, strlen(name)}); }

py_Name py_namev(c11_sv name) {
    uint16_t index = c11_smallmap_s2n__get(&_interned, name, 0);
    if(index != 0) return index;
    // generate new index
    if(_interned.length > 65530) c11__abort("py_Name index overflow");
    // NOTE: we must allocate the string in the heap so iterators are not invalidated
    char* p = PK_MALLOC(name.size + 1);
    memcpy(p, name.data, name.size);
    p[name.size] = '\0';
    RInternedEntry entry;
    entry.data = p;
    entry.size = name.size;
    entry.ref = NULL;
    c11_vector__push(RInternedEntry, &_r_interned, entry);
    index = _r_interned.length;  // 1-based
    // save to _interned
    c11_smallmap_s2n__set(&_interned, (c11_sv){p, name.size}, index);
    assert(_interned.length == _r_interned.length);
    return index;
}

const char* py_name2str(py_Name index) {
    assert(index > 0 && index <= _interned.length);
    return c11__getitem(RInternedEntry, &_r_interned, index - 1).data;
}

c11_sv py_name2sv(py_Name index) {
    assert(index > 0 && index <= _interned.length);
    RInternedEntry entry = c11__getitem(RInternedEntry, &_r_interned, index - 1);
    return (c11_sv){entry.data, entry.size};
}

py_GlobalRef py_name2ref(py_Name index) {
    assert(index > 0 && index <= _interned.length);
    RInternedEntry entry = c11__getitem(RInternedEntry, &_r_interned, index - 1);
    if(entry.ref == NULL){
        entry.ref = PK_MALLOC(16);  // ...
        c11_sv sv;
        sv.data = entry.data;
        sv.size = entry.size;
        py_newstrv(entry.ref, sv);
    }
    return entry.ref;
}
