#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pocketpy.h"

char* read_file(const char* path) {
    FILE* file = fopen(path, "r");
    if(file == NULL) {
        printf("Error: file not found\n");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, file);
    buffer[size] = 0;
    return buffer;
}

int main(int argc, char** argv) {
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    py_initialize();
    const char* source = "1 < 2";

    if(py_eval(source)) {
        // handle the result
        bool _L0 = py_tobool(py_lastretval());
        printf("%d\n", _L0);
    }

    py_Ref r0 = py_reg(0);
    py_Ref r1 = py_reg(1);

    py_newint(r0, 1);
    py_newfloat(r1, 2.5);

    bool ok = py_binaryadd(r0, r1);
    assert(ok);
    double res = py_tofloat(py_lastretval());
    printf("%f\n", res);

    py_finalize();
    return 0;

    //     if(argc != 2) goto __HELP;
    //     char* source = read_file(argv[1]);
    //     py_initialize();

    //     if(py_exec(source)){
    //         py_Error* err = py_getlasterror();
    //         py_Error__print(err);
    //     }

    //     py_finalize();
    //     free(source);

    // __HELP:
    //     printf("Usage: pocketpy [filename]\n");
    //     return 0;
}
