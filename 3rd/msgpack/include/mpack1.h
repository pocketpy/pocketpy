#pragma once

#include "mpack.h"
#include "pocketpy.h"

typedef bool (*is_mpack_ext_t)(py_Type type);
typedef bool (*py_to_mpack_ext_t)(py_Ref object, mpack_writer_t* writer);
typedef bool (*mpack_to_py_ext_t)(py_StackRef tmp, mpack_node_t node);

void mpack_config_exttype_callbacks(is_mpack_ext_t is_ext,
                                    py_to_mpack_ext_t to_mpack,
                                    mpack_to_py_ext_t to_py);