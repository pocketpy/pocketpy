#pragma once

#include <stdint.h>
#include "pocketpy/common/str.h"

#ifdef __cplusplus
extern "C" {
#endif

uint16_t py_name2(c11_sv name);
c11_sv py_name2sv(uint16_t index);

void py_Name__initialize();
void py_Name__finalize();

#ifdef __cplusplus
}
#endif
