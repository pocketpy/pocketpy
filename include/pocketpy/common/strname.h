#pragma once

#include <stdint.h>
#include "pocketpy/common/str.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t pkpy_StrName__map(c11_string name);
c11_string pkpy_StrName__rmap(uint16_t index);

void pkpy_StrName__initialize();
void pkpy_StrName__finalize();

#ifdef __cplusplus
}
#endif