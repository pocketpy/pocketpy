#include "pocketpy/common/strname.h"
#include "pocketpy/common/smallmap.h"
#include "pocketpy/common/utils.h"
#include "pocketpy/common/vector.h"

#include <stdio.h>

// TODO: use a more efficient data structure
static c11_smallmap_s2n _interned;
static c11_vector/*T=char* */ _r_interned;
static bool _initialized = false;

void pkpy_StrName__initialize(){
    if(_initialized) return;
    c11_smallmap_s2n__ctor(&_interned);
    for(int i=0; i<_r_interned.count; i++){
        free(c11__at(char*, &_r_interned, i));
    }
    c11_vector__ctor(&_r_interned, sizeof(c11_string));
    _initialized = true;
}

void pkpy_StrName__finalize(){
    if(!_initialized) return;
    c11_smallmap_s2n__dtor(&_interned);
    c11_vector__dtor(&_r_interned);
}

uint16_t pkpy_StrName__map(c11_string name){
    // TODO: PK_GLOBAL_SCOPE_LOCK()
    if(!_initialized){
        pkpy_StrName__initialize(); // lazy init
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

c11_string pkpy_StrName__rmap(uint16_t index){
    assert(_initialized);
    assert(index > 0 && index <= _interned.count);
    char* p = c11__getitem(char*, &_r_interned, index - 1);
    return (c11_string){p, strlen(p)};
}
