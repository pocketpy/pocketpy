#ifdef OPCODE

/**************************/
OPCODE(NO_OP)
/**************************/
OPCODE(POP_TOP)
OPCODE(DUP_TOP)
OPCODE(DUP_TOP_TWO)
OPCODE(ROT_TWO)
OPCODE(ROT_THREE)
OPCODE(PRINT_EXPR)
/**************************/
OPCODE(LOAD_CONST)
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
/**************************/
OPCODE(LOAD_SMALL_INT)
/**************************/
OPCODE(LOAD_ELLIPSIS)
OPCODE(LOAD_FUNCTION)
OPCODE(LOAD_NULL)
/**************************/
OPCODE(LOAD_FAST)
OPCODE(LOAD_NAME)
OPCODE(LOAD_NONLOCAL)
OPCODE(LOAD_GLOBAL)
OPCODE(LOAD_ATTR)
OPCODE(LOAD_CLASS_GLOBAL)
OPCODE(LOAD_METHOD)
OPCODE(LOAD_SUBSCR)

OPCODE(STORE_FAST)
OPCODE(STORE_NAME)
OPCODE(STORE_GLOBAL)
OPCODE(STORE_ATTR)
OPCODE(STORE_SUBSCR)

OPCODE(DELETE_FAST)
OPCODE(DELETE_NAME)
OPCODE(DELETE_GLOBAL)
OPCODE(DELETE_ATTR)
OPCODE(DELETE_SUBSCR)
/**************************/
OPCODE(BUILD_LONG)
OPCODE(BUILD_IMAG)
OPCODE(BUILD_BYTES)
OPCODE(BUILD_TUPLE)
OPCODE(BUILD_LIST)
OPCODE(BUILD_DICT)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)
OPCODE(BUILD_STRING)
/**************************/
OPCODE(BINARY_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)
/**************************/
OPCODE(JUMP_FORWARD)
OPCODE(POP_JUMP_IF_FALSE)
OPCODE(POP_JUMP_IF_TRUE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)
OPCODE(SHORTCUT_IF_FALSE_OR_POP)
OPCODE(LOOP_CONTINUE)
OPCODE(LOOP_BREAK)
/***/
OPCODE(JUMP_ABSOLUTE_TOP)
OPCODE(GOTO)
/**************************/
OPCODE(REPR)
OPCODE(CALL)
OPCODE(CALL_VARGS)
OPCODE(RETURN_VALUE)
OPCODE(YIELD_VALUE)
/**************************/
OPCODE(LIST_APPEND)
OPCODE(DICT_ADD)
OPCODE(SET_ADD)
/**************************/
OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)
OPCODE(UNARY_STAR)
OPCODE(UNARY_INVERT)
/**************************/
OPCODE(GET_ITER)
OPCODE(FOR_ITER)
/**************************/
OPCODE(IMPORT_PATH)
OPCODE(POP_IMPORT_STAR)
/**************************/
OPCODE(UNPACK_SEQUENCE)
OPCODE(UNPACK_EX)
/**************************/
OPCODE(BEGIN_CLASS)
OPCODE(END_CLASS)
OPCODE(STORE_CLASS_ATTR)
OPCODE(ADD_CLASS_ANNOTATION)
/**************************/
OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)
/**************************/
OPCODE(TRY_ENTER)
OPCODE(EXCEPTION_MATCH)
OPCODE(RAISE)
OPCODE(RAISE_ASSERT)
OPCODE(RE_RAISE)
OPCODE(PUSH_EXCEPTION)
OPCODE(POP_EXCEPTION)
/**************************/
OPCODE(FSTRING_EVAL)
OPCODE(FORMAT_STRING)
/**************************/
#endif
