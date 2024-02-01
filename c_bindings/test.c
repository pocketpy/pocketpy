#include "pocketpy_c.h"
#include <stdio.h>
#include <stdlib.h>

//tests the c bindings for pocketpy

void check_impl(pkpy_vm* vm, bool result, int lineno) {
    if (!result) {
        printf("ERROR: failed where it should have succeed at line %i\n", lineno);
        char* message;
        if (!pkpy_clear_error(vm, &message)) {
            printf("clear error reported everything was fine\n");
            exit(1);
        }

        printf("%s\n", message);
        free(message);
        exit(1);
    }
}

void fail_impl(pkpy_vm* vm, bool result, int lineno) {
    if (result) {
        printf("ERROR: succeeded where it should have failed line %i\n", lineno);
        exit(1);
    } else {
        char* message;
        if (pkpy_clear_error(vm, &message)) {
            printf("actually errored! line %i\n", lineno);
            free(message);
            exit(1);
        }
    }
}

void error_impl(pkpy_vm* vm, bool result, int lineno) {
    if (result) {
        printf("ERROR: succeeded where it should have failed line %i\n", lineno);
        exit(1);
    } else {
        char* message;
        if (!pkpy_clear_error(vm, &message)){
            printf("clear error reported everything was fine\n");
            exit(1);
        } else {
            printf("successfully errored with this message: \n");
            printf("%s\n", message);
            free(message);
        }
    }
}


#define check(r) check_impl(vm, (r), __LINE__)
#define fail(r) fail_impl(vm, (r), __LINE__)
#define error(r) error_impl(vm, (r), __LINE__)

int test_binding(pkpy_vm* vm) {
    pkpy_push_int(vm, 12);
    return 1;
}

int test_multiple_return(pkpy_vm* vm) {
    pkpy_push_int(vm, 12);
    pkpy_push_int(vm, 13);
    return 2;
}

int test_minus(pkpy_vm* vm) {
    int a, b;
    pkpy_to_int(vm, 0, &a);
    pkpy_to_int(vm, 1, &b);
    pkpy_push_int(vm, a - b);
    return 1;
}

int test_fib(pkpy_vm* vm) {
    int n;
    pkpy_to_int(vm, 0, &n);
    if (n == 1) {
        pkpy_push_int(vm, n);
    } else {
        pkpy_getglobal(vm, pkpy_name("test_fib"));
        pkpy_push_null(vm);
        pkpy_push_int(vm, n-1);
        pkpy_vectorcall(vm, 1);
        int r_int;
        pkpy_to_int(vm, -1, &r_int);
        pkpy_pop_top(vm);
        pkpy_push_int(vm, r_int + n);
    }
    return 1;
}

int test_default_argument(pkpy_vm* vm){
    int x;
    pkpy_to_int(vm, -1, &x);
    bool ok = x == 5;
    pkpy_push_bool(vm, ok);
    return 1;
}

int test_return_none(pkpy_vm* vm) {
    return 0;
}

int test_error_propagate(pkpy_vm* vm) {
    pkpy_error(vm, "NameError", pkpy_string("catch me"));
    return 1;
}

int test_nested_error(pkpy_vm* vm) {
    pkpy_getglobal(vm, pkpy_name("error_from_python"));
    pkpy_push_null(vm);
    pkpy_vectorcall(vm, 0);
    return 0;
}

#define PRINT_TITLE(x) printf("\n====== %s ======\n", x)

int main(int argc, char** argv) {
    pkpy_vm* vm = pkpy_new_vm(true);

    PRINT_TITLE("test basic exec");
    check(pkpy_exec(vm, "print('hello world!')"));
    fail(pkpy_getglobal(vm, pkpy_name("nonexistatn")));

    // test int methods
    PRINT_TITLE("test int methods");
    int r_int;
    check(pkpy_push_int(vm, 11));
    pkpy_CName m_eleven = pkpy_name("eleven");
    check(pkpy_setglobal(vm, m_eleven));
    check(pkpy_exec(vm, "print(eleven)"));
    check(pkpy_getglobal(vm, m_eleven));
    check(pkpy_is_int(vm, -1));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("%i\n", r_int);                      // 11
    printf("%i\n", pkpy_stack_size(vm));        // 1

    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    PRINT_TITLE("test float methods");
    double r_float;
    check(pkpy_push_float(vm, 11.125));
    pkpy_CName m_elevenf = pkpy_name("elevenf");
    check(pkpy_setglobal(vm, m_elevenf));
    check(pkpy_exec(vm, "print(elevenf)"));
    check(pkpy_getglobal(vm, m_elevenf));
    check(pkpy_is_float(vm, -1));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("%.3f\n", r_float);
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    PRINT_TITLE("test bool methods");
    bool r_bool;
    check(pkpy_push_bool(vm, false));
    pkpy_CName m_false_test = pkpy_name("false_test");
    check(pkpy_setglobal(vm, m_false_test));
    check(pkpy_exec(vm, "print(false_test)"));
    check(pkpy_getglobal(vm, m_false_test));
    check(pkpy_is_bool(vm, -1));
    check(pkpy_to_bool(vm, -1, &r_bool));
    printf("%i\n", r_bool);
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    PRINT_TITLE("test string methods");
    pkpy_CString r_string;
    check(pkpy_push_string(vm, pkpy_string("hello!")));
    check(pkpy_setglobal(vm, pkpy_name("hello1")));
    check(pkpy_exec(vm, "print(hello1)"));
    check(pkpy_push_string(vm, pkpy_string("hello!")));
    check(pkpy_is_string(vm, -1));
    check(pkpy_to_string(vm, -1, &r_string));
    
    puts(r_string);

    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    PRINT_TITLE("test none methods");
    check(pkpy_push_none(vm));
    pkpy_CName m_none = pkpy_name("none");
    check(pkpy_setglobal(vm, m_none));
    check(pkpy_exec(vm, "print(none)"));
    check(pkpy_getglobal(vm, m_none));
    check(pkpy_is_none(vm, -1));
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    PRINT_TITLE("test voidp methods");
    void* vp = (void*) 123;
    check(pkpy_push_voidp(vm, vp));
    check(pkpy_setglobal(vm, pkpy_name("vp")));
    check(pkpy_exec(vm, "print(vp)"));
    check(pkpy_getglobal(vm, pkpy_name("vp")));
    check(pkpy_is_voidp(vm, -1));
    vp = NULL;
    check(pkpy_to_voidp(vm, -1, &vp));
    printf("%i\n", (int) (intptr_t) vp);
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));

    PRINT_TITLE("test sizing and indexing");
    int stack_size = pkpy_stack_size(vm);
    printf("stack size %i\n", stack_size);
    check(pkpy_is_int(vm, 0));
    check(pkpy_is_float(vm, 1));
    check(pkpy_is_bool(vm, 2));
    check(pkpy_is_string(vm, 3));
    check(pkpy_is_none(vm, 4));
    check(pkpy_is_voidp(vm, 5));
    check(pkpy_is_int(vm, -6));
    check(pkpy_is_float(vm, -5));
    check(pkpy_is_bool(vm, -4));
    check(pkpy_is_string(vm, -3));
    check(pkpy_is_none(vm, -2));
    check(pkpy_is_voidp(vm, -1));

    PRINT_TITLE("test error catching");
    error(pkpy_exec(vm, "let's make sure syntax errors get caught"));
    //stack should be cleared after error is resolved
    check(pkpy_stack_size(vm) == 0);

    PRINT_TITLE("test simple call");
    check(pkpy_exec(vm, "def x(x, y) : return x - y"));
    check(pkpy_getglobal(vm, pkpy_name("x")));
    check(pkpy_push_null(vm));
    check(pkpy_push_int(vm, 2));
    check(pkpy_push_int(vm, 3));
    check(pkpy_vectorcall(vm, 2));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("x : %i\n", r_int);

    PRINT_TITLE("test vararg call");
    check(pkpy_exec(vm, "def vararg_x(*x) : return sum(x)"));
    check(pkpy_getglobal(vm, pkpy_name("vararg_x")));
    check(pkpy_push_null(vm));
    check(pkpy_push_int(vm, 1));
    check(pkpy_push_int(vm, 2));
    check(pkpy_push_int(vm, 3));
    check(pkpy_push_int(vm, 4));
    check(pkpy_push_int(vm, 5));
    check(pkpy_push_int(vm, 6));
    check(pkpy_vectorcall(vm, 6));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("vararg_x : %i\n", r_int);

    PRINT_TITLE("test keyword call");
    check(pkpy_exec(vm, "def keyword_x(x=1, y=1) : return x+y"));
    check(pkpy_getglobal(vm, pkpy_name("keyword_x")));
    check(pkpy_push_null(vm));
    check(pkpy_push_int(vm, 3));
    check(pkpy_vectorcall(vm, 1));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("keyword_x : %i\n", r_int);      // 3+1

    check(pkpy_getglobal(vm, pkpy_name("keyword_x")));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("keyword_x : %i\n", r_int);      // 1+1
    check(pkpy_stack_size(vm) == 4);
    check(pkpy_pop(vm, 4));     // clear stack

    PRINT_TITLE("test return many");
    check(pkpy_exec(vm, "def retmany_x() : return 1, 2, 3"));
    check(pkpy_getglobal(vm, pkpy_name("retmany_x")));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));

    check(pkpy_stack_size(vm) == 1);
    check(pkpy_unpack_sequence(vm, 3));
    check(pkpy_stack_size(vm) == 3);

    check(pkpy_to_int(vm, -3, &r_int));
    printf("retmany_x : %i\n", r_int);
    check(pkpy_to_int(vm, -2, &r_int));
    printf("retmany_x : %i\n", r_int);
    check(pkpy_to_int(vm, -1, &r_int));
    printf("retmany_x : %i\n", r_int);

    // test argument error
    check(pkpy_getglobal(vm, pkpy_name("x")));
    check(pkpy_push_null(vm));
    error(pkpy_vectorcall(vm, 0));

    check(pkpy_exec(vm, "l = []"));
    check(pkpy_getglobal(vm, pkpy_name("l")));
    check(pkpy_get_unbound_method(vm, pkpy_name("append")));
    check(pkpy_push_string(vm, pkpy_string("hello")));
    check(pkpy_vectorcall(vm, 1));
    check(pkpy_pop_top(vm));        // pop None returned by append()
    check(pkpy_exec(vm, "print(l)"));

    PRINT_TITLE("test bindings");
    check(pkpy_push_function(vm, "test_binding()", test_binding));
    check(pkpy_setglobal(vm, pkpy_name("test_binding")));
    check(pkpy_exec(vm, "print(test_binding())"));
    check(pkpy_stack_size(vm) == 0);

    check(pkpy_push_function(vm, "test_multiple_return()", test_multiple_return));
    check(pkpy_setglobal(vm, pkpy_name("test_multiple_return")));
    check(pkpy_stack_size(vm) == 0);

    check(pkpy_push_function(vm, "test_default_argument(x=5)", test_default_argument));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));
    check(pkpy_stack_size(vm) == 1);
    check(pkpy_is_bool(vm, -1) == true);
    check(pkpy_to_bool(vm, -1, &r_bool));
    check(r_bool == true);
    check(pkpy_pop_top(vm));
    check(pkpy_stack_size(vm) == 0);

    PRINT_TITLE("test bindings 2");
    check(pkpy_push_function(vm, "test_minus(a, b)", test_minus));
    check(pkpy_setglobal(vm, pkpy_name("test_minus")));
    check(pkpy_exec(vm, "print(test_minus(5, 3))"));
    check(pkpy_exec(vm, "for i in range(5): print(test_minus(5, i))"));
    check(pkpy_stack_size(vm) == 0);

    PRINT_TITLE("test bindings fib");
    check(pkpy_push_function(vm, "test_fib(n: int) -> int", test_fib));
    check(pkpy_setglobal(vm, pkpy_name("test_fib")));
    check(pkpy_exec(vm, "print(test_fib(10))"));
    check(pkpy_stack_size(vm) == 0);

    PRINT_TITLE("test error propagate");
    check(pkpy_push_function(vm, "test_error_propagate()", test_error_propagate));
    check(pkpy_setglobal(vm, pkpy_name("test_error_propagate")));
    error(pkpy_exec(vm, "test_error_propagate()"));

    check(pkpy_getglobal(vm, pkpy_name("test_multiple_return")));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));
    check(pkpy_stack_size(vm) == 1);
    check(pkpy_unpack_sequence(vm, 2));
    check(pkpy_stack_size(vm) == 2);
    check(pkpy_pop(vm, 2));
    check(pkpy_stack_size(vm) == 0);

    PRINT_TITLE("test other errors");
    check(pkpy_getglobal(vm, pkpy_name("test_error_propagate")));
    check(pkpy_pop_top(vm));
    fail(pkpy_getglobal(vm, pkpy_name("nonexistant")));
    error(pkpy_exec(vm, "raise NameError('testing error throwing from python')"));

    PRINT_TITLE("test TypeError");
    check(pkpy_push_float(vm, 2.0));
    error(pkpy_to_int(vm, -1, &r_int));

    PRINT_TITLE("test complicated errors");
    pkpy_exec(vm, "test_error_propagate()");
    check(pkpy_check_error(vm));
    pkpy_clear_error(vm, NULL);

    //this should be catchable
    check(pkpy_exec(vm, "try : test_error_propagate(); except NameError : pass"));
    error(pkpy_error(vm, "Exception", pkpy_string("test direct error mechanism")));

    //more complicated error handling
    check(pkpy_exec(vm, "def error_from_python() : raise NotImplementedError()"));
    check(pkpy_push_function(vm, "test_nested_error()", test_nested_error));
    check(pkpy_setglobal(vm, pkpy_name("test_nested_error")));
    error(pkpy_exec(vm, "test_nested_error()"));

    PRINT_TITLE("test getattr/setattr");
    check(pkpy_stack_size(vm) == 0);
    check(pkpy_exec(vm, "import math"));
    check(pkpy_getglobal(vm, pkpy_name("math")));
    check(pkpy_getattr(vm, pkpy_name("pi")));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("pi: %.2f\n", (float)r_float);
    check(pkpy_pop(vm, 1));

    // math.pi = 2
    check(pkpy_push_int(vm, 2));
    check(pkpy_eval(vm, "math"));
    check(pkpy_setattr(vm, pkpy_name("pi")));
    check(pkpy_exec(vm, "print(math.pi)"));

    PRINT_TITLE("test eval");
    check(pkpy_eval(vm, "math.pi"));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("pi: %.2f\n", (float)r_float);
    check(pkpy_pop(vm, 1));
    check(pkpy_stack_size(vm) == 0);

    PRINT_TITLE("test py_repr");
    check(pkpy_eval(vm, "['1', 2, (3, '4')]"));
    check(pkpy_py_repr(vm));
    check(pkpy_to_string(vm, -1, &r_string));
    
    puts(r_string);
    check(pkpy_pop_top(vm));
    check(pkpy_stack_size(vm) == 0);
    return 0;
}
