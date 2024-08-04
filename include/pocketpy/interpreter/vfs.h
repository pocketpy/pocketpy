#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/common/str.h"
#include "pocketpy/pocketpy.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VfsEntry {
    bool is_file;

    union {
        struct {
            int size;
            unsigned char* data;
        } _file;

        c11_vector _dir;
    };
} VfsEntry;

#define SMALLMAP_T__HEADER
#define K c11_sv
#define V VfsEntry
#define NAME VfsDir
#define less(a, b) (c11_sv__cmp((a), (b)) < 0)
#define equal(a, b) (c11_sv__cmp((a), (b)) == 0)
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__HEADER

typedef struct Vfs {
    VfsEntry root;
} Vfs;

void Vfs__ctor(Vfs* self);
void Vfs__dtor(Vfs* self);

#ifdef __cplusplus
}
#endif