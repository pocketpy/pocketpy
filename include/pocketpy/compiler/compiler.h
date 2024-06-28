#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/compiler/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

Error* pk_compile(pk_SourceData_ src);


#ifdef __cplusplus
}
#endif