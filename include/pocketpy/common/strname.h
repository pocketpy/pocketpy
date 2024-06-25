#pragma once

#include <stdint.h>
#include "pocketpy/common/str.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t StrName;

#define py_name(name) pk_StrName__map(#name)

uint16_t pk_StrName__map(const char*);
uint16_t pk_StrName__map2(c11_string);
const char* pk_StrName__rmap(uint16_t index);

void pk_StrName__initialize();
void pk_StrName__finalize();

#ifdef __cplusplus
}
#endif

/* global names */
#ifdef __cplusplus
extern "C" {
namespace pkpy {
#endif

// unary operators
extern uint16_t __repr__;
extern uint16_t __str__;
extern uint16_t __hash__;
extern uint16_t __len__;
extern uint16_t __iter__;
extern uint16_t __next__;
extern uint16_t __neg__;
// logical operators
extern uint16_t __eq__;
extern uint16_t __lt__;
extern uint16_t __le__;
extern uint16_t __gt__;
extern uint16_t __ge__;
extern uint16_t __contains__;
// binary operators
extern uint16_t __add__;
extern uint16_t __radd__;
extern uint16_t __sub__;
extern uint16_t __rsub__;
extern uint16_t __mul__;
extern uint16_t __rmul__;
extern uint16_t __truediv__;
extern uint16_t __floordiv__;
extern uint16_t __mod__;
extern uint16_t __pow__;
extern uint16_t __matmul__;
extern uint16_t __lshift__;
extern uint16_t __rshift__;
extern uint16_t __and__;
extern uint16_t __or__;
extern uint16_t __xor__;
extern uint16_t __invert__;
// indexer
extern uint16_t __getitem__;
extern uint16_t __setitem__;
extern uint16_t __delitem__;

// specials
extern uint16_t __new__;
extern uint16_t __init__;
extern uint16_t __call__;
extern uint16_t __divmod__;
extern uint16_t __enter__;
extern uint16_t __exit__;
extern uint16_t __name__;
extern uint16_t __all__;
extern uint16_t __package__;
extern uint16_t __path__;
extern uint16_t __class__;
extern uint16_t __missing__;

extern uint16_t pk_id_add;
extern uint16_t pk_id_set;
extern uint16_t pk_id_long;
extern uint16_t pk_id_complex;

#ifdef __cplusplus
}   // namespace pkpy
}   // extern "C"
#endif
