#include "pocketpy/objects/base.h"

/* predefined vars */
const pkpy_Type tp_object = 1, tp_type = 2;
const pkpy_Type tp_int = 3, tp_float = 4, tp_bool = 5, tp_str = 6;
const pkpy_Type tp_list = 7, tp_tuple = 8;
const pkpy_Type tp_slice = 9, tp_range = 10, tp_module = 11;
const pkpy_Type tp_function = 12, tp_native_func = 13, tp_bound_method = 14;
const pkpy_Type tp_super = 15, tp_exception = 16, tp_bytes = 17, tp_mappingproxy = 18;
const pkpy_Type tp_dict = 19, tp_property = 20, tp_star_wrapper = 21;
const pkpy_Type tp_staticmethod = 22, tp_classmethod = 23;
const pkpy_Type tp_none_type = 24, tp_not_implemented_type = 25;
const pkpy_Type tp_ellipsis = 26;
const pkpy_Type tp_op_call = 27, tp_op_yield = 28;

PyVar pkpy_True = {.type=tp_bool, .is_ptr=false, ._bool=true};
PyVar pkpy_False = {.type=tp_bool, .is_ptr=false, ._bool=false};
PyVar pkpy_None = {.type=tp_none_type, .is_ptr=false};
PyVar pkpy_NotImplemented = {.type=tp_not_implemented_type, .is_ptr=false};
PyVar pkpy_Ellipsis = {.type=tp_ellipsis, .is_ptr=false};
PyVar pkpy_NULL = {.type=0, .is_ptr=false};
PyVar pkpy_OP_CALL = {.type=tp_op_call, .is_ptr=false};
PyVar pkpy_OP_YIELD = {.type=tp_op_yield, .is_ptr=false};
