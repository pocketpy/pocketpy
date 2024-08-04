#include "pocketpy/interpreter/vfs.h"
#include "pocketpy/interpreter/vm.h"

#define SMALLMAP_T__SOURCE
#define K c11_sv
#define V VfsEntry
#define NAME VfsDir
#define less(a, b) (c11_sv__cmp((a), (b)) < 0)
#define equal(a, b) (c11_sv__cmp((a), (b)) == 0)
#include "pocketpy/xmacros/smallmap.h"
#undef SMALLMAP_T__SOURCE

static VfsEntry* Vfs__get(const char* path) {
    c11_vector /*T=c11_sv*/ cpnts = c11_sv__split((c11_sv){path, strlen(path)}, '/');

    VfsEntry* root = &pk_current_vm->__vfs.root;
    for(int i = 0; i < cpnts.count; i++) {
        c11_sv cpnt = c11__getitem(c11_sv, &cpnts, i);
        VfsEntry* entry = VfsDir__try_get(&root->_dir, cpnt);
        if(entry == NULL) {
            c11_vector__dtor(&cpnts);
            return NULL;
        }
        if(entry->is_file) {
            VfsEntry* retval = i == cpnts.count - 1 ? entry : NULL;
            c11_vector__dtor(&cpnts);
            return retval;
        } else {
            root = entry;
        }
    }
    c11_vector__dtor(&cpnts);
    return root;
}

static void VfsDir__delete_recursively(VfsDir* self) {
    for(int i = 0; i < self->count; i++) {
        VfsDir_KV* kv = c11__at(VfsDir_KV, self, i);
        free((char*)kv->key.data);
        if(kv->value.is_file) {
            free(kv->value._file.data);
        } else {
            VfsDir__delete_recursively(&kv->value._dir);
        }
    }
    VfsDir__dtor(self);
}

void Vfs__ctor(Vfs* self) {
    self->root.is_file = false;
    VfsDir__ctor(&self->root._dir);
}

void Vfs__dtor(Vfs* self) { VfsDir__delete_recursively(&self->root._dir); }

unsigned char* py_vfsread(const char* path, int* size) {
    VfsEntry* entry = Vfs__get(path);
    if(entry == NULL || !entry->is_file) return NULL;
    *size = entry->_file.size;
    unsigned char* retval = malloc(*size);
    memcpy(retval, entry->_file.data, *size);
    return retval;
}

static void VfsDir__dupset(VfsDir* self, c11_sv key, VfsEntry value) {
    char* p = malloc(key.size);
    memcpy(p, key.data, key.size);
    VfsDir__set(self, (c11_sv){p, key.size}, value);
}

bool py_vfswrite(const char* path, unsigned char* data, int size) {
    c11_vector /*T=c11_sv*/ cpnts = c11_sv__split((c11_sv){path, strlen(path)}, '/');
    VfsEntry* root = &pk_current_vm->__vfs.root;
    for(int i = 0; i < cpnts.count; i++) {
        c11_sv cpnt = c11__getitem(c11_sv, &cpnts, i);
        VfsEntry* entry = VfsDir__try_get(&root->_dir, cpnt);
        if(entry == NULL) {
            if(i == cpnts.count - 1) {
                // create file
                VfsEntry entry = {
                    .is_file = true,
                    ._file.size = size,
                    ._file.data = data,
                };
                VfsDir__dupset(&root->_dir, cpnt, entry);
                c11_vector__dtor(&cpnts);
                return true;
            } else {
                // create missing directory
                VfsEntry entry = {
                    .is_file = false,
                };
                VfsDir__ctor(&entry._dir);
                VfsDir__dupset(&root->_dir, cpnt, entry);
            }
        } else {
            if(i == cpnts.count - 1) {
                if(!entry->is_file) break;
                // update file
                free(entry->_file.data);
                entry->_file.size = size;
                entry->_file.data = data;
                c11_vector__dtor(&cpnts);
                return true;
            } else {
                if(entry->is_file) break;
                root = entry;
            }
        }
    }
    c11_vector__dtor(&cpnts);
    return false;
}

char** py_vfslist(const char* path, int* length) {
    VfsEntry* entry = Vfs__get(path);
    if(entry == NULL || entry->is_file) return NULL;
    *length = 0;
    char** ret = malloc(sizeof(char*) * entry->_dir.count);
    for(int i = 0; i < entry->_dir.count; i++) {
        VfsDir_KV* child = c11__at(VfsDir_KV, &entry->_dir, i);
        if(child->value.is_file) {
            int size = child->key.size;
            ret[i] = malloc(size + 1);
            memcpy(ret[i], child->key.data, size);
            ret[i][size] = '\0';
            (*length)++;
        }
    }
    return ret;
}