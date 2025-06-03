#pragma once

#include "pocketpy/pocketpy.h"

void pk_names_initialize();
void pk_names_finalize();

#define MAGIC_METHOD(x) extern py_Name x;
#include "pocketpy/xmacros/magics.h"
#undef MAGIC_METHOD