#pragma once

void pk__add_module_os();
void pk__add_module_sys();
void pk__add_module_io();

void pk__add_module_math();
void pk__add_module_dis();
void pk__add_module_random();
void pk__add_module_json();
void pk__add_module_gc();
void pk__add_module_time();
void pk__add_module_easing();
void pk__add_module_traceback();
void pk__add_module_enum();
void pk__add_module_inspect();
void pk__add_module_pickle();
void pk__add_module_base64();
void pk__add_module_importlib();
void pk__add_module_unicodedata();

void pk__add_module_vmath();
void pk__add_module_array2d();
void pk__add_module_colorcvt();

void pk__add_module_conio();
void pk__add_module_lz4();
void pk__add_module_pkpy();

#ifdef PK_BUILD_MODULE_LIBHV
void pk__add_module_libhv();
#else
#define pk__add_module_libhv()
#endif

#ifdef PK_BUILD_MODULE_CUTE_PNG
void pk__add_module_cute_png();
#else
#define pk__add_module_cute_png()
#endif
