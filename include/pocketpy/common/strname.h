#pragma once

#include <stdint.h>
#include "pocketpy/pocketpy.h"
#include "pocketpy/common/str.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t StrName;

#define py_name(name) pk_StrName__map(name)

uint16_t pk_StrName__map(const char*);
uint16_t pk_StrName__map2(c11_string);
const char* pk_StrName__rmap(uint16_t index);
c11_string pk_StrName__rmap2(uint16_t index);

void pk_StrName__initialize();
void pk_StrName__finalize();

extern uint16_t pk_id_add;
extern uint16_t pk_id_set;
extern uint16_t pk_id_long;
extern uint16_t pk_id_complex;

#ifdef __cplusplus
}
#endif
