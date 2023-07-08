#ifndef PK_EXPORT

#ifdef _WIN32
#define PK_EXPORT __declspec(dllexport)
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define PK_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define PK_EXPORT __attribute__((visibility("default")))
#endif

#define PK_INLINE_EXPORT PK_EXPORT inline

#endif

#ifdef PK_SHARED_MODULE
#undef PK_INLINE_EXPORT
#define PK_INLINE_EXPORT inline
#endif
