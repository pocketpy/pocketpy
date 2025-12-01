''' 
========= src/common/algorithm.c =========
'''

# 该函数的return false没有被覆盖
# bool c11__stable_sort(void* ptr_,
#                       int length,
#                       int elem_size,
#                       int (*f_lt)(const void* a, const void* b, void* extra),
#                       void* extra) {
#     // merge sort
#     char *ptr = ptr_, *tmp = PK_MALLOC(length * elem_size);
#     for(int seg = 1; seg < length; seg *= 2) {
#         for(char* a = ptr; a < ptr + (length - seg) * elem_size; a += 2 * seg * elem_size) {
#             char *b = a + seg * elem_size, *a_end = b, *b_end = b + seg * elem_size;
#             if(b_end > ptr + length * elem_size) b_end = ptr + length * elem_size;
#             bool ok = _stable_sort_merge(a, a_end, b, b_end, tmp, elem_size, f_lt, extra);
#             if(!ok) {
#                 PK_FREE(tmp);
# ---->           return false;  
#             }
#             memcpy(a, tmp, b_end - a);
#         }
#     }
#     PK_FREE(tmp);
#     return true;
# }


# 调查得知只要触发bool ok = pk_stack_binaryop(vm, __lt__, __gt__)的return false即可
# bool pk_stack_binaryop(VM* self, py_Name op, py_Name rop) {
#     // [a, b]
#     py_Ref magic = py_tpfindmagic(SECOND()->type, op);
#     if(magic) {
#         bool ok = py_call(magic, 2, SECOND());
# ---->   if(!ok) return false;
#         if(self->last_retval.type != tp_NotImplementedType) return true;
#     }
#     // try reverse operation
#     if(rop) {
#         // [a, b] -> [b, a]
#         py_TValue tmp = *TOP();
#         *TOP() = *SECOND();
#         *SECOND() = tmp;
#         magic = py_tpfindmagic(SECOND()->type, rop);
#         if(magic) {
#             bool ok = py_call(magic, 2, SECOND());
# ---->       if(!ok) return false;
#             if(self->last_retval.type != tp_NotImplementedType) return true;
#         }
#     }
#     // eq/ne op never fails
#     bool res = py_isidentical(SECOND(), TOP());
#     if(op == __eq__) {
#         py_newbool(py_retval(), res);
#         return true;
#     }
#     if(op == __ne__) {
#         py_newbool(py_retval(), !res);
#         return true;
#     }

#     py_Type lhs_t = rop ? TOP()->type : SECOND()->type;
#     py_Type rhs_t = rop ? SECOND()->type : TOP()->type;
#     return TypeError("unsupported operand type(s) for '%s': '%t' and '%t'",
#                      pk_op2str(op),
#                      lhs_t,
#                      rhs_t);
# }

# 
# 调查得知只要触发bool ok = py_call(py_tpfindmagic(SECOND()->type, __lt__/__gt__), 2, SECOND())的return false即可
# bool py_call(py_Ref f, int argc, py_Ref argv) {
#     if(f->type == tp_nativefunc) {
#         return py_callcfunc(f->_cfunc, argc, argv);
#     } else {
#         py_push(f);
#         py_pushnil();
#         for(int i = 0; i < argc; i++)
#             py_push(py_offset(argv, i));
#         bool ok = py_vectorcall(argc, 0);
#         return ok;
#     }
# }

class A():
    def __lt__(self, other:int=1):
        return True


try:
    [A(), 1111].sort()
except TypeError:
    pass