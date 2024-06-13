#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief A python value in pocketpy.
 */
typedef struct {
    int type;
    int _0, _1, _2;
} pkpy_Var;

/**
 * @brief Check if the pkpy_Var is null.
 * @param self The variable to check.
 * @return True if the variable is null, false otherwise.
 */
#define pkpy_Var__is_null(self) ((self)->type == 0)

/**
 * @brief Set the variable to null.
 * @param self The variable to set.
 */
#define pkpy_Var__set_null(self) do { (self)->type = 0; } while(0)

/**
 * @brief Check if two pkpy_Vars are equal, respects to __eq__ method.
 * @param vm The virtual machine.
 * @param a The first pkpy_Var.
 * @param b The second pkpy_Var.
 * @return True if the pkpy_Vars are equal, false otherwise.
 */
bool pkpy_Var__eq__(void *vm, pkpy_Var a, pkpy_Var b);

/**
 * @brief Get the hash of the pkpy_Var, respects to __hash__ method.
 * @param vm The virtual machine.
 * @param a The pkpy_Var to hash.
 * @return The hash of the pkpy_Var.
 */
int64_t pkpy_Var__hash__(void *vm, pkpy_Var a);

#ifdef __cplusplus
}
#endif
