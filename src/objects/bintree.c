#include "pocketpy/objects/bintree.h"

#include "pocketpy/common/vector.h"
#include "pocketpy/objects/object.h"

void BinTree__ctor(BinTree* self, void* key, py_Ref value, const BinTreeConfig* config) {
    self->key = key;
    self->value = *value;
    self->config = config;
    self->left = NULL;
    self->right = NULL;
}

void BinTree__dtor(BinTree* self) {
    if(self->config->need_free_key) PK_FREE(self->key);
    if(self->left) {
        BinTree__dtor(self->left);
        PK_FREE(self->left);
    }
    if(self->right) {
        BinTree__dtor(self->right);
        PK_FREE(self->right);
    }
}

void BinTree__set(BinTree* self, void* key, py_Ref value) {
    int cmp = self->config->f_cmp(key, self->key);
    if(cmp < 0) {
        if(self->left) {
            BinTree__set(self->left, key, value);
        } else {
            self->left = PK_MALLOC(sizeof(BinTree));
            BinTree__ctor(self->left, key, value, self->config);
        }
    } else if(cmp > 0) {
        if(self->right) {
            BinTree__set(self->right, key, value);
        } else {
            self->right = PK_MALLOC(sizeof(BinTree));
            BinTree__ctor(self->right, key, value, self->config);
        }
    } else {
        self->value = *value;
    }
}

py_Ref BinTree__try_get(BinTree* self, void* key) {
    int cmp = self->config->f_cmp(key, self->key);
    if(cmp < 0) {
        if(self->left) {
            return BinTree__try_get(self->left, key);
        } else {
            return NULL;
        }
    } else if(cmp > 0) {
        if(self->right) {
            return BinTree__try_get(self->right, key);
        } else {
            return NULL;
        }
    } else {
        return &self->value;
    }
}

bool BinTree__contains(BinTree* self, void* key) { return BinTree__try_get(self, key) != NULL; }

void BinTree__apply_mark(BinTree* self, c11_vector* p_stack) {
    pk__mark_value(&self->value);
    if(self->left) BinTree__apply_mark(self->left, p_stack);
    if(self->right) BinTree__apply_mark(self->right, p_stack);
}