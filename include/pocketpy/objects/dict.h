#pragma once

#include <stdbool.h>
#include "pocketpy/objects/pyvar.h"
#include "pocketpy/common/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief `pkpy_Dict` is the Dict type in Python */
typedef struct {
    int count;             /** number of elements in the dictionary */
    c11_vector _entries;   /** contains `pkpy_DictEntry` (hidden type) */
    int _htcap;            /** capacity of the hashtable, always a power of 2 */
    void* _hashtable;      /** contains indecies, can be `u8`, `u16` or `u32` according to size*/
} pkpy_Dict;

/** @brief `pkpy_DictIter` is used to iterate over a `pkpy_Dict` */
typedef struct {
    const pkpy_Dict* _dict;
    int _index;
} pkpy_DictIter;

/**
 * @brief `pkpy_Dict` constructor
 * @param self `pkpy_Dict` instance
 */
void pkpy_Dict__ctor(pkpy_Dict* self);

/**
 * @brief `pkpy_Dict` destructor
 * @param self `pkpy_Dict` instance
 */
void pkpy_Dict__dtor(pkpy_Dict* self);

/**
 * @brief Copy a `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @return a new `pkpy_Dict` instance, must be destructed by the caller
 */
pkpy_Dict pkpy_Dict__copy(const pkpy_Dict* self);

/**
 * @brief Set a key-value pair into the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @param vm __eq__ and __hash__ context
 * @param key key to set
 * @param val value to set
 * @return `true` if the key is newly added, `false` if the key already exists
 */
bool pkpy_Dict__set(pkpy_Dict* self, void* vm, pkpy_Var key, pkpy_Var val);

/**
 * @brief Check if a key exists in the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @param vm __eq__ and __hash__ context
 * @param key key to check
 * @return `true` if the key exists, `false` otherwise
 */
bool pkpy_Dict__contains(const pkpy_Dict* self, void* vm, pkpy_Var key);

/**
 * @brief Remove a key from the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @param vm __eq__ and __hash__ context
 * @param key key to remove
 * @return `true` if the key was found and removed, `false` if the key doesn't exist
 */
bool pkpy_Dict__del(pkpy_Dict* self, void* vm, pkpy_Var key);

/**
 * @brief Try to get a value from the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @param vm __eq__ and __hash__ context
 * @param key key to get
 * @return the value associated with the key, `NULL` if the key doesn't exist
 */
const pkpy_Var* pkpy_Dict__try_get(const pkpy_Dict* self, void* vm, pkpy_Var key);

/**
 * @brief Update the `pkpy_Dict` with another one
 * @param self `pkpy_Dict` instance
 * @param vm __eq__ and __hash__ context
 * @param other `pkpy_Dict` instance to update with
 */
void pkpy_Dict__update(pkpy_Dict* self, void *vm, const pkpy_Dict* other);

/**
 * @brief Clear the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 */
void pkpy_Dict__clear(pkpy_Dict* self);

/**
 * @brief Iterate over the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @return an iterator over the `pkpy_Dict`
 */
pkpy_DictIter pkpy_Dict__iter(const pkpy_Dict* self);

/**
 * @brief Iterate over the `pkpy_Dict`
 * @param self `pkpy_Dict` instance
 * @param key key will be filled with the current key, can be `NULL` if not needed
 * @param value value will be filled with the current value, can be `NULL` if not needed
 * @return `true` if the iteration is still valid, `false` otherwise
 */
bool pkpy_DictIter__next(pkpy_DictIter* self, pkpy_Var* key, pkpy_Var* value);

#ifdef __cplusplus
}
#endif
