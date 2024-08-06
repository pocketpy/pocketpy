#pragma once

#include "pocketpy/common/vector.h"
#include "pocketpy/compiler/lexer.h"
#include "pocketpy/objects/sourcedata.h"
#include "pocketpy/objects/codeobject.h"

Error* pk_compile(SourceData_ src, CodeObject* out);
