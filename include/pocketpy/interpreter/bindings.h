#pragma once

#include "pocketpy/pocketpy.h"

bool generator__next__(int argc, py_Ref argv);
bool array2d_like_iterator__next__(int argc, py_Ref argv);
bool list_iterator__next__(int argc, py_Ref argv);
bool tuple_iterator__next__(int argc, py_Ref argv);
bool dict_items__next__(int argc, py_Ref argv);
bool range_iterator__next__(int argc, py_Ref argv);
bool str_iterator__next__(int argc, py_Ref argv);