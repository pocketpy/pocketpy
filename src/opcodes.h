#ifdef OPCODE

// Do nothing
OPCODE(NO_OP)

// This op is a placeholder that should never be executed
OPCODE(DELETED_OP)

// Load a constant from the `co_consts`
// ARG: array index
OPCODE(LOAD_CONST)

OPCODE(IMPORT_NAME)
OPCODE(PRINT_EXPR)
OPCODE(POP_TOP)
OPCODE(CALL)
OPCODE(RETURN_VALUE)

OPCODE(BINARY_OP)
OPCODE(COMPARE_OP)
OPCODE(BITWISE_OP)
OPCODE(IS_OP)
OPCODE(CONTAINS_OP)

OPCODE(UNARY_NEGATIVE)
OPCODE(UNARY_NOT)

OPCODE(DUP_TOP)

OPCODE(BUILD_LIST)
OPCODE(BUILD_MAP)
OPCODE(BUILD_SET)
OPCODE(BUILD_SLICE)

OPCODE(LIST_APPEND)

OPCODE(GET_ITER)
OPCODE(FOR_ITER)

OPCODE(POP_JUMP_IF_FALSE)
OPCODE(JUMP_ABSOLUTE)
OPCODE(SAFE_JUMP_ABSOLUTE)
OPCODE(JUMP_IF_TRUE_OR_POP)
OPCODE(JUMP_IF_FALSE_OR_POP)

// non-standard python opcodes
OPCODE(LOAD_NONE)
OPCODE(LOAD_TRUE)
OPCODE(LOAD_FALSE)
OPCODE(LOAD_EVAL_FN)        // load eval() callable into stack
OPCODE(LOAD_LAMBDA)         // LOAD_CONST + set __module__ attr
OPCODE(LOAD_ELLIPSIS)

OPCODE(ASSERT)
OPCODE(RAISE_ERROR)

OPCODE(STORE_FUNCTION)
OPCODE(BUILD_CLASS)

OPCODE(LOAD_NAME_REF)       // no arg
OPCODE(BUILD_ATTR_REF)      // arg for the name_ptr, [ptr, name_ptr] -> (*ptr).name_ptr
OPCODE(BUILD_INDEX_REF)     // no arg, [ptr, expr] -> (*ptr)[expr]
OPCODE(STORE_NAME_REF)      // arg for the name_ptr, [expr], directly store to the name_ptr without pushing it to the stack
OPCODE(STORE_REF)           // no arg, [ptr, expr] -> *ptr = expr
OPCODE(DELETE_REF)          // no arg, [ptr] -> [] -> delete ptr

OPCODE(BUILD_SMART_TUPLE)   // if all elements are pointers, build a compound pointer, otherwise build a tuple
OPCODE(BUILD_STRING)        // arg is the expr count, build a string from the top of the stack

OPCODE(GOTO)

OPCODE(WITH_ENTER)
OPCODE(WITH_EXIT)

OPCODE(JUMP_RELATIVE)

OPCODE(RAISE_VARARGS)

#endif