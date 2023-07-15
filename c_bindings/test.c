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

int main(int argc, char** argv) {
    pkpy_vm* vm = pkpy_new_vm(true);

    check(pkpy_exec(vm, "print('hello world!')"));
    error(pkpy_getglobal(vm, pkpy_name("nonexistatn")));

    printf("\ntesting int methods\n");
    int r_int;
    check(pkpy_push_int(vm, 11));
    pkpy_CName m_eleven = pkpy_name("eleven");
    check(pkpy_setglobal(vm, m_eleven));
    check(pkpy_exec(vm, "print(eleven)"));
    check(pkpy_getglobal(vm, m_eleven));
    check(pkpy_is_int(vm, -1));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("%i\n", r_int);
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting float methods\n");
    float r_float;
    check(pkpy_push_float(vm, 11.11));
    pkpy_CName m_elevenf = pkpy_name("elevenf");
    check(pkpy_setglobal(vm, m_elevenf));
    check(pkpy_exec(vm, "print(elevenf)"));
    check(pkpy_getglobal(vm, m_elevenf));
    check(pkpy_is_float(vm, -1));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("%f\n", r_float);
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting bool methods\n");
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

    printf("\ntesting string methods\n");
    pkpy_CString r_string;
    check(pkpy_push_string(vm, pkpy_string("hello!")));
    check(pkpy_setglobal(vm, pkpy_name("hello1")));
    check(pkpy_exec(vm, "print(hello1)"));
    check(pkpy_push_string(vm, pkpy_string("hello!")));
    check(pkpy_is_string(vm, -1));
    check(pkpy_to_string(vm, -1, &r_string));
    
    for(int i=0; i<r_string.size; i++) {
        putchar(r_string.data[i]);
    }
    putchar('\n');

    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting None methods\n");
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

    printf("\ntesting voidp methods\n");
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


    printf("\ntesting sizing and indexing\n");
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
    
    printf("\ntesting error catching\n");
    error(pkpy_exec(vm, "let's make sure syntax errors get caught"));
    check(pkpy_stack_size(vm) == 0); //stack should be cleared after error is resolved

    printf("\ntesting calls\n");

    check(pkpy_exec(vm, "def x(x, y) : return x - y"));
    check(pkpy_exec(vm, "def vararg_x(*x) : return sum(x)"));
    check(pkpy_exec(vm, "def keyword_x(x=1, y=1) : return x+y"));
    check(pkpy_exec(vm, "def retmany_x() : return 1, 2, 3"));

    check(pkpy_getglobal(vm, pkpy_name("x")));
    check(pkpy_push_null(vm));
    check(pkpy_push_int(vm, 2));
    check(pkpy_push_int(vm, 3));
    check(pkpy_vectorcall(vm, 2));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("x : %i\n", r_int);

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

    check(pkpy_getglobal(vm, pkpy_name("keyword_x")));
    check(pkpy_push_null(vm));
    check(pkpy_push_int(vm, 3));
    check(pkpy_vectorcall(vm, 1));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("keyword_x : %i\n", r_int);

    check(pkpy_getglobal(vm, pkpy_name("keyword_x")));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("keyword_x : %i\n", r_int);

    check(pkpy_stack_size(vm) == 4);

    check(pkpy_getglobal(vm, pkpy_name("retmany_x")));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));
    check(pkpy_stack_size(vm) == 7);
    check(pkpy_to_int(vm, -3, &r_int));
    printf("retmany_x : %i\n", r_int);
    check(pkpy_to_int(vm, -2, &r_int));
    printf("retmany_x : %i\n", r_int);
    check(pkpy_to_int(vm, -1, &r_int));
    printf("retmany_x : %i\n", r_int);

    check(pkpy_getglobal(vm, pkpy_name("x")));
    check(pkpy_push_null(vm));
    error(pkpy_vectorcall(vm, 0));

    check(pkpy_exec(vm, "l = []"));

    check(pkpy_getglobal(vm, pkpy_name("l")));
    check(pkpy_get_unbound_method(vm, pkpy_name("append")));
    check(pkpy_push_string(vm, pkpy_string("hello")));
    check(pkpy_vectorcall(vm, 1));
    check(pkpy_exec(vm, "print(l)"));


    printf("\ntesting pushing functions\n");

    check(pkpy_push_function(vm, "test_binding()", test_binding));
    check(pkpy_setglobal(vm, pkpy_name("test_binding")));
    check(pkpy_exec(vm, "print(test_binding())"));

    check(pkpy_push_function(vm, "test_multiple_return()", test_multiple_return));
    check(pkpy_setglobal(vm, pkpy_name("test_multiple_return")));

    //uncomment if _exec changes
    //check(pkpy_exec(vm, "test_multiple_return()"));
    //check(pkpy_stack_size(vm) == 2);


    check(pkpy_push_function(vm, "test_error_propagate()", test_error_propagate));
    check(pkpy_setglobal(vm, pkpy_name("test_error_propagate")));
    error(pkpy_exec(vm, "test_error_propagate()"));

    check(pkpy_getglobal(vm, pkpy_name("test_multiple_return")));
    check(pkpy_push_null(vm));
    check(pkpy_vectorcall(vm, 0));
    check(pkpy_stack_size(vm) == 2);

    
    check(pkpy_pop(vm, 2));
    check(pkpy_stack_size(vm) == 0);

    check(pkpy_getglobal(vm, pkpy_name("test_error_propagate")));
    check(pkpy_pop_top(vm));
    fail(pkpy_getglobal(vm, pkpy_name("nonexistant")));

    error(pkpy_exec(vm, "raise NameError('testing error throwing from python')"));

    pkpy_exec(vm, "test_error_propagate()");
    check(pkpy_check_error(vm));
    // testing code going to standard error, can ignore next error
    pkpy_clear_error(vm, NULL);

    //errors
    //this should be catchable
    check(pkpy_exec(vm, "try : test_error_propagate(); except NameError : pass"));

    error(pkpy_error(vm, "_", pkpy_string("test direct error mechanism")));


    //more complicated error handling
    check(pkpy_exec(vm, "def error_from_python() : raise NotImplementedError()"));
    check(pkpy_push_function(vm, "test_nested_error()", test_nested_error));
    check(pkpy_setglobal(vm, pkpy_name("test_nested_error")));
    error(pkpy_exec(vm, "test_nested_error()"));

    check(pkpy_exec(vm, "import math"));
    check(pkpy_getglobal(vm, pkpy_name("math")));
    check(pkpy_getattr(vm, pkpy_name("pi")));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("pi: %.2f\n", r_float);

    check(pkpy_eval(vm, "math.pi"));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("pi: %.2f\n", r_float);

    check(pkpy_pop(vm, 1));

    // math.pi = 2
    check(pkpy_push_int(vm, 2));
    check(pkpy_eval(vm, "math"));
    check(pkpy_setattr(vm, pkpy_name("pi")));
    check(pkpy_exec(vm, "print(math.pi)"));


    //should give a type error
    check(pkpy_push_float(vm, 2.0));
    error(pkpy_to_int(vm, -1, &r_int));

    return 0;
}
