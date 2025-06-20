#pragma once

#include "pocketpy/objects/base.h"
#include "pocketpy/common/vector.h"
#include "pocketpy/pocketpy.h"

#define FIXEDHASH_T__HEADER
#define K py_Name
#define V py_TValue
#define NAME CachedNames
#include "pocketpy/xmacros/fixedhash.h"
#undef FIXEDHASH_T__HEADER