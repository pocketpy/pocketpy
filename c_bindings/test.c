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
    pkpy_get_global(vm, "does not exist");
    return 1;
}

int test_nested_error(pkpy_vm* vm) {
    pkpy_get_global(vm, "error_from_python");
    pkpy_call(vm, 0);
    return 0;
}


pkpy_vm* vm;

int main(int argc, char** argv) {

    vm = pkpy_vm_create(true, true);

    //test run
    check(pkpy_vm_run(vm, "print('hello world!')"));

    error(pkpy_get_global(vm, "nonexistatn"));

    printf("\ntesting int methods\n");
    int r_int;
    check(pkpy_push_int(vm, 11));
    check(pkpy_set_global(vm, "eleven"));
    check(pkpy_vm_run(vm, "print(eleven)"));
    check(pkpy_get_global(vm, "eleven"));
    check(pkpy_is_int(vm, -1));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("%i\n", r_int);
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting float methods\n");
    double r_float;
    check(pkpy_push_float(vm, 11.11));
    check(pkpy_set_global(vm, "elevenf"));
    check(pkpy_vm_run(vm, "print(elevenf)"));
    check(pkpy_get_global(vm, "elevenf"));
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
    check(pkpy_set_global(vm, "false_test"));
    check(pkpy_vm_run(vm, "print(false_test)"));
    check(pkpy_get_global(vm, "false_test"));
    check(pkpy_is_bool(vm, -1));
    check(pkpy_to_bool(vm, -1, &r_bool));
    printf("%i\n", r_bool);
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting string methods\n");
    char* r_string;
    check(pkpy_push_string(vm, "hello!"));
    check(pkpy_set_global(vm, "hello1"));
    check(pkpy_vm_run(vm, "print(hello1)"));
    check(pkpy_push_stringn(vm, "hello!", 5));
    check(pkpy_is_string(vm, -1));
    check(pkpy_to_string(vm, -1, &r_string));
    printf("%s\n", r_string);
    free(r_string);
    const char* r_stringn;
    int r_size;
    check(pkpy_to_stringn(vm, -1, &r_stringn, &r_size));
    printf("%.*s\n", r_size, r_stringn);
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_none(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting None methods\n");
    check(pkpy_push_none(vm));
    check(pkpy_set_global(vm, "none"));
    check(pkpy_vm_run(vm, "print(none)"));
    check(pkpy_get_global(vm, "none"));
    check(pkpy_is_none(vm, -1));
    fail(pkpy_is_int(vm, -1));
    fail(pkpy_is_float(vm, -1));
    fail(pkpy_is_bool(vm, -1));
    fail(pkpy_is_string(vm, -1));
    fail(pkpy_is_voidp(vm, -1));

    printf("\ntesting voidp methods\n");
    void* vp = (void*) 123;
    check(pkpy_push_voidp(vm, vp));
    check(pkpy_set_global(vm, "vp"));
    check(pkpy_vm_run(vm, "print(vp)"));
    check(pkpy_get_global(vm, "vp"));
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
    check(pkpy_check_stack(vm, 10));
    check(pkpy_check_stack(vm, 26));
    fail(pkpy_check_stack(vm, 27));
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
    check(pkpy_push(vm, -3));
    check(pkpy_is_string(vm, -1));
    
    printf("\ntesting error catching\n");
    error(pkpy_vm_run(vm, "let's make sure syntax errors get caught"));
    check(pkpy_stack_size(vm) == 0); //stack should be cleared after error is resolved

    printf("\ntesting calls\n");

    check(pkpy_vm_run(vm, "def x(x, y) : return x - y"));
    check(pkpy_vm_run(vm, "def vararg_x(*x) : return sum(x)"));
    check(pkpy_vm_run(vm, "def keyword_x(x=1, y=1) : return x+y"));
    check(pkpy_vm_run(vm, "def retmany_x() : return 1, 2, 3"));

    check(pkpy_get_global(vm, "x"));
    check(pkpy_push_int(vm, 2));
    check(pkpy_push_int(vm, 3));
    check(pkpy_call(vm, 2));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("x : %i\n", r_int);

    check(pkpy_get_global(vm, "vararg_x"));
    check(pkpy_push_int(vm, 1));
    check(pkpy_push_int(vm, 2));
    check(pkpy_push_int(vm, 3));
    check(pkpy_push_int(vm, 4));
    check(pkpy_push_int(vm, 5));
    check(pkpy_push_int(vm, 6));
    check(pkpy_call(vm, 6));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("vararg_x : %i\n", r_int);

    check(pkpy_get_global(vm, "keyword_x"));
    check(pkpy_push_int(vm, 3));
    check(pkpy_call(vm, 1));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("keyword_x : %i\n", r_int);

    check(pkpy_get_global(vm, "keyword_x"));
    check(pkpy_call(vm, 0));
    check(pkpy_to_int(vm, -1, &r_int));
    printf("keyword_x : %i\n", r_int);

    check(pkpy_stack_size(vm) == 4);

    check(pkpy_get_global(vm, "retmany_x"));
    check(pkpy_call(vm, 0));
    check(pkpy_stack_size(vm) == 7);
    check(pkpy_to_int(vm, -3, &r_int));
    printf("retmany_x : %i\n", r_int);
    check(pkpy_to_int(vm, -2, &r_int));
    printf("retmany_x : %i\n", r_int);
    check(pkpy_to_int(vm, -1, &r_int));
    printf("retmany_x : %i\n", r_int);

    check(pkpy_get_global(vm, "x"));
    error(pkpy_call(vm, 0));

    check(pkpy_vm_run(vm, "l = []"));

    check(pkpy_get_global(vm, "l"));
    check(pkpy_push_string(vm, "hello"));
    check(pkpy_call_method(vm, "append", 1));
    check(pkpy_vm_run(vm, "print(l)"));


    printf("\ntesting pushing functions\n");

    check(pkpy_push_function(vm, test_binding, 0));
    check(pkpy_set_global(vm, "test_binding"));
    check(pkpy_vm_run(vm, "print(test_binding())"));

    check(pkpy_push_function(vm, test_multiple_return, 0));
    check(pkpy_set_global(vm, "test_multiple_return"));

    //uncomment if _exec changes
    //check(pkpy_vm_run(vm, "test_multiple_return()"));
    //check(pkpy_stack_size(vm) == 2);


    check(pkpy_push_function(vm, test_error_propagate, 0));
    check(pkpy_set_global(vm, "test_error_propagate"));
    error(pkpy_vm_run(vm, "test_error_propagate()"));

    check(pkpy_get_global(vm, "test_multiple_return"));
    check(pkpy_call(vm, 0));
    check(pkpy_stack_size(vm) == 2);

    
    check(pkpy_pop(vm, 2));
    check(pkpy_stack_size(vm) == 0);

    check(pkpy_check_global(vm, "test_error_propagate"));
    fail(pkpy_check_global(vm, "nonexistant"));

    error(pkpy_vm_run(vm, "raise NameError('testing error throwing from python')"));

    pkpy_vm_run(vm, "test_error_propagate()");
    check(pkpy_check_error(vm));
    // testing code going to standard error, can ignore next error
    pkpy_clear_error(vm, NULL);

    //with the current way execptions are handled, this will fail and pass the
    //error clean through, ignoring the python handling
    //
    //maybe worth fixing someday, but for now it is functionating as implemented
    check(pkpy_vm_run(vm, "try : test_error_propagate(); except NameError : pass"));

    error(pkpy_error(vm, "_", "test direct error mechanism"));


    //more complicated error handling
    //
    //at the moment this is disabled, as this use case is not supported
    //it will cause the program to abort 
    //
    //a python exception thrown from python can not pass through a c binding
    //
    //this means for now the api can only be used to make shallow bindings, or
    //else care must be taken to not let an exception go through a c binding by
    //catching it in python first 
    //
    //at such a time this interferes with a real world use case of the bindings
    //we can revisit it
    //
    check(pkpy_vm_run(vm, "def error_from_python() : raise NotImplementedError()"));
    check(pkpy_push_function(vm, test_nested_error, 0));
    check(pkpy_set_global(vm, "test_nested_error"));
    error(pkpy_vm_run(vm, "test_nested_error()"));

    check(pkpy_vm_run(vm, "import math"));
    check(pkpy_get_global(vm, "math"));
    check(pkpy_getattr(vm, "pi"));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("pi: %.2f\n", r_float);

    check(pkpy_eval(vm, "math.pi"));
    check(pkpy_to_float(vm, -1, &r_float));
    printf("pi: %.2f\n", r_float);

    check(pkpy_pop(vm, 1));

    // math.pi = 2
    check(pkpy_push_int(vm, 2));
    check(pkpy_eval(vm, "math"));
    check(pkpy_setattr(vm, "pi"));
    check(pkpy_vm_run(vm, "print(math.pi)"));

    pkpy_vm_destroy(vm);
    return 0;
}
