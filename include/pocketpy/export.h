#ifndef PK_EXPORT

#ifdef _WIN32
#define PK_EXPORT __declspec(dllexport)
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define PK_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define PK_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#endif

#define PK_LEGACY_EXPORT PK_EXPORT inline

#endif
