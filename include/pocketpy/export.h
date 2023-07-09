#ifndef PK_EXPORT

#ifdef _WIN32
#define PK_EXPORT __declspec(dllexport)
#elif __EMSCRIPTEN__
#include <emscripten.h>
#define PK_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define PK_EXPORT __attribute__((visibility("default")))
#endif

#endif


#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#elif __EMSCRIPTEN__

#include <emscripten.h>

#elif __unix__

#include <dlfcn.h>

#endif