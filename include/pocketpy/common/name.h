#pragma once

#include "pocketpy/pocketpy.h"

#ifdef __cplusplus
extern "C" {
#endif

void pk_names_initialize();
void pk_names_finalize();

#define MAGIC_METHOD(x) extern py_Name x;
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD

py_Name py_namev(c11_sv name);
c11_sv py_name2sv(py_Name index);
py_Name py_name(const char* name);
const char* py_name2str(py_Name index);

#ifdef __cplusplus
}  // extern "C"
#endif
