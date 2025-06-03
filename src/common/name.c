#include "pocketpy/common/name.h"
#include <stdatomic.h>

typedef struct InternedEntry InternedEntry;
typedef struct InternedEntry{
    InternedEntry* prev;
    InternedEntry* next;
    py_i64 hash;
    int size;       // size of the data excluding the null-terminator
    char data[];    // null-terminated data
} InternedEntry;

typedef struct {
    InternedEntry* table[1 << 16];
    atomic_bool lock;
} InternedNames;

void pk_names_initialize() {

}


void pk_names_finalize() {
    
}