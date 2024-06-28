#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/codeobject.h"

#ifdef __cplusplus
extern "C" {
#endif

Error* pk_compile(pk_SourceData_ src, CodeObject* out);

void pk_Compiler__initialize();
#define pk_Compiler__finalize()  // do nothing

#ifdef __cplusplus
}
#endif