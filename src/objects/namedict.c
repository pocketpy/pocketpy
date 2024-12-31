#include "pocketpy/objects/namedict.h"

#define SMALLMAP_T__SOURCE
#define K uint16_t
#define V py_TValue
#define NAME NameDict
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE

void ModuleDict__ctor(ModuleDict* self, const char* path, py_TValue module) {
    self->path = path;
    self->module = module;
    self->left = NULL;
    self->right = NULL;
}

void ModuleDict__dtor(ModuleDict* self) {
    if(self->left) {
        ModuleDict__dtor(self->left);
        PK_FREE(self->left);
    }
    if(self->right) {
        ModuleDict__dtor(self->right);
        PK_FREE(self->right);
    }
}

void ModuleDict__set(ModuleDict* self, const char* key, py_TValue val) {
    if(self->path == NULL) {
        self->path = key;
        self->module = val;
    }
    int cmp = strcmp(key, self->path);
    if(cmp < 0) {
        if(self->left) {
            ModuleDict__set(self->left, key, val);
        } else {
            self->left = PK_MALLOC(sizeof(ModuleDict));
            ModuleDict__ctor(self->left, key, val);
        }
    } else if(cmp > 0) {
        if(self->right) {
            ModuleDict__set(self->right, key, val);
        } else {
            self->right = PK_MALLOC(sizeof(ModuleDict));
            ModuleDict__ctor(self->right, key, val);
        }
    } else {
        self->module = val;
    }
}

py_TValue* ModuleDict__try_get(ModuleDict* self, const char* path) {
    if(self->path == NULL) return NULL;
    int cmp = strcmp(path, self->path);
    if(cmp < 0) {
        if(self->left) {
            return ModuleDict__try_get(self->left, path);
        } else {
            return NULL;
        }
    } else if(cmp > 0) {
        if(self->right) {
            return ModuleDict__try_get(self->right, path);
        } else {
            return NULL;
        }
    } else {
        return &self->module;
    }
}

bool ModuleDict__contains(ModuleDict* self, const char* path) {
    return ModuleDict__try_get(self, path) != NULL;
}