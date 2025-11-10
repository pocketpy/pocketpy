#include "pocketpy.h"

#include "mpack.h"
#include <assert.h>

static bool mpack_to_py(mpack_node_t node) {
    py_StackRef tmp = py_pushtmp();

    mpack_type_t type = mpack_node_type(node);

    switch(type) {
        case mpack_type_nil: py_newnone(tmp); break;

        case mpack_type_bool: py_newbool(tmp, mpack_node_bool(node)); break;

        case mpack_type_int: py_newint(tmp, mpack_node_i64(node)); break;

        case mpack_type_uint: py_newint(tmp, (int64_t)mpack_node_u64(node)); break;

        case mpack_type_float: py_newfloat(tmp, mpack_node_float(node)); break;

        case mpack_type_double: py_newfloat(tmp, mpack_node_double(node)); break;

        case mpack_type_str: {
            const char* str = mpack_node_str(node);
            size_t len = mpack_node_strlen(node);
            c11_sv sv = {str, (int)len};
            py_newstrv(tmp, sv);
            break;
        }

        case mpack_type_bin: {
            const char* data = mpack_node_bin_data(node);
            size_t len = mpack_node_bin_size(node);
            unsigned char* byte_data = py_newbytes(tmp, len);
            memcpy(byte_data, data, len);
            break;
        }

        case mpack_type_array: {
            size_t count = mpack_node_array_length(node);
            py_newlistn(tmp, count);
            for(int i = 0; i < count; i++) {
                mpack_node_t child = mpack_node_array_at(node, i);
                bool ok = mpack_to_py(child);
                if(!ok) return false;
                py_list_setitem(tmp, i, py_peek(-1));
                py_pop();
            }
            break;
        }

        case mpack_type_map: {
            size_t count = mpack_node_map_count(node);
            py_newdict(tmp);
            for(size_t i = 0; i < count; i++) {
                mpack_node_t key_node = mpack_node_map_key_at(node, i);
                mpack_node_t val_node = mpack_node_map_value_at(node, i);
                if(mpack_node_type(key_node) != mpack_type_str) {
                    return TypeError("msgpack: key must be strings");
                }
                if(!mpack_to_py(key_node)) return false;
                if(!mpack_to_py(val_node)) return false;
                bool ok = py_dict_setitem(tmp, py_peek(-2), py_peek(-1));
                if(!ok) return false;
                py_shrink(2);
            }
            break;
        }
        default: return ValueError("msgpack: invalid node type");
    }
    return true;
}

static bool msgpack_loads(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int size;
    unsigned char* data = py_tobytes(argv, &size);

    mpack_tree_t tree;
    mpack_tree_init_data(&tree, (const char*)data, size);
    mpack_tree_parse(&tree);

    if(mpack_tree_error(&tree) != mpack_ok) {
        mpack_tree_destroy(&tree);
        return ValueError("msgpack: parse error");
    }

    mpack_node_t node = mpack_tree_root(&tree);
    bool ok = mpack_to_py(node);
    mpack_tree_destroy(&tree);
    if(!ok) return false;

    py_assign(py_retval(), py_peek(-1));
    py_pop();
    return true;
}

// static mpack_node_t py_to_mpack(py_Ref object) {

// }

static bool msgpack_dumps(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    return ValueError("not yet");
}

void pk__add_module_msgpack() {
    py_GlobalRef mod = py_newmodule("msgpack");

    py_bindfunc(mod, "loads", msgpack_loads);
    py_bindfunc(mod, "dumps", msgpack_dumps);
}