#include "pocketpy/common/strname.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/common/vector.h"

#include <stdio.h>

// TODO: use a more efficient data structure
static c11_smallmap_s2n _interned;
static c11_vector/*T=char* */ _r_interned;
static bool _initialized = false;

void pk_StrName__initialize(){
    if(_initialized) return;
    c11_smallmap_s2n__ctor(&_interned);
    for(int i=0; i<_r_interned.count; i++){
        free(c11__at(char*, &_r_interned, i));
    }
    c11_vector__ctor(&_r_interned, sizeof(c11_string));
    _initialized = true;

    // unary operators
    __repr__ = pk_StrName__map("__repr__");
    __str__ = pk_StrName__map("__str__");
    __hash__ = pk_StrName__map("__hash__");
    __len__ = pk_StrName__map("__len__");
    __iter__ = pk_StrName__map("__iter__");
    __next__ = pk_StrName__map("__next__");
    __neg__ = pk_StrName__map("__neg__");
    // logical operators
    __eq__ = pk_StrName__map("__eq__");
    __lt__ = pk_StrName__map("__lt__");
    __le__ = pk_StrName__map("__le__");
    __gt__ = pk_StrName__map("__gt__");
    __ge__ = pk_StrName__map("__ge__");
    __contains__ = pk_StrName__map("__contains__");
    // binary operators
    __add__ = pk_StrName__map("__add__");
    __radd__ = pk_StrName__map("__radd__");
    __sub__ = pk_StrName__map("__sub__");
    __rsub__ = pk_StrName__map("__rsub__");
    __mul__ = pk_StrName__map("__mul__");
    __rmul__ = pk_StrName__map("__rmul__");
    __truediv__ = pk_StrName__map("__truediv__");
    __floordiv__ = pk_StrName__map("__floordiv__");
    __mod__ = pk_StrName__map("__mod__");
    __pow__ = pk_StrName__map("__pow__");
    __matmul__ = pk_StrName__map("__matmul__");
    __lshift__ = pk_StrName__map("__lshift__");
    __rshift__ = pk_StrName__map("__rshift__");
    __and__ = pk_StrName__map("__and__");
    __or__ = pk_StrName__map("__or__");
    __xor__ = pk_StrName__map("__xor__");
    __invert__ = pk_StrName__map("__invert__");
    // indexer
    __getitem__ = pk_StrName__map("__getitem__");
    __setitem__ = pk_StrName__map("__setitem__");
    __delitem__ = pk_StrName__map("__delitem__");

    // specials
    __new__ = pk_StrName__map("__new__");
    __init__ = pk_StrName__map("__init__");
    __call__ = pk_StrName__map("__call__");
    __divmod__ = pk_StrName__map("__divmod__");
    __enter__ = pk_StrName__map("__enter__");
    __exit__ = pk_StrName__map("__exit__");
    __name__ = pk_StrName__map("__name__");
    __all__ = pk_StrName__map("__all__");
    __package__ = pk_StrName__map("__package__");
    __path__ = pk_StrName__map("__path__");
    __class__ = pk_StrName__map("__class__");
    __missing__ = pk_StrName__map("__missing__");

    pk_id_add = pk_StrName__map("add");
    pk_id_set = pk_StrName__map("set");
    pk_id_long = pk_StrName__map("long");
    pk_id_complex = pk_StrName__map("complex");
}

void pk_StrName__finalize(){
    if(!_initialized) return;
    c11_smallmap_s2n__dtor(&_interned);
    c11_vector__dtor(&_r_interned);
}

uint16_t pk_StrName__map(const char* name){
    return pk_StrName__map2((c11_string){name, strlen(name)});
}

uint16_t pk_StrName__map2(c11_string name){
    // TODO: PK_GLOBAL_SCOPE_LOCK()
    if(!_initialized){
        pk_StrName__initialize(); // lazy init
    }
    uint16_t index = c11_smallmap_s2n__get(&_interned, name, 0);
    if(index != 0) return index;
    // generate new index
    if(_interned.count > 65530){
        PK_FATAL_ERROR("StrName index overflow\n");
    }
    // NOTE: we must allocate the string in the heap so iterators are not invalidated
    char* p = malloc(name.size + 1);
    memcpy(p, name.data, name.size);
    p[name.size] = '\0';
    c11_vector__push(char*, &_r_interned, p);
    index = _r_interned.count;  // 1-based
    // save to _interned
    c11_smallmap_s2n__set(&_interned, (c11_string){p, name.size}, index);
    assert(_interned.count == _r_interned.count);
    return index;
}

const char* pk_StrName__rmap(uint16_t index){
    assert(_initialized);
    assert(index > 0 && index <= _interned.count);
    return c11__getitem(char*, &_r_interned, index - 1);
}

c11_string pk_StrName__rmap2(uint16_t index){
    const char* p = pk_StrName__rmap(index);
    return (c11_string){p, strlen(p)};
}



// unary operators
uint16_t __repr__;
uint16_t __str__;
uint16_t __hash__;
uint16_t __len__;
uint16_t __iter__;
uint16_t __next__;
uint16_t __neg__;
// logical operators
uint16_t __eq__;
uint16_t __lt__;
uint16_t __le__;
uint16_t __gt__;
uint16_t __ge__;
uint16_t __contains__;
// binary operators
uint16_t __add__;
uint16_t __radd__;
uint16_t __sub__;
uint16_t __rsub__;
uint16_t __mul__;
uint16_t __rmul__;
uint16_t __truediv__;
uint16_t __floordiv__;
uint16_t __mod__;
uint16_t __pow__;
uint16_t __matmul__;
uint16_t __lshift__;
uint16_t __rshift__;
uint16_t __and__;
uint16_t __or__;
uint16_t __xor__;
uint16_t __invert__;
// indexer
uint16_t __getitem__;
uint16_t __setitem__;
uint16_t __delitem__;

// specials
uint16_t __new__;
uint16_t __init__;
uint16_t __call__;
uint16_t __divmod__;
uint16_t __enter__;
uint16_t __exit__;
uint16_t __name__;
uint16_t __all__;
uint16_t __package__;
uint16_t __path__;
uint16_t __class__;
uint16_t __missing__;

uint16_t pk_id_add;
uint16_t pk_id_set;
uint16_t pk_id_long;
uint16_t pk_id_complex;
