#include <stdio.h>
#include <stdlib.h>

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
    const char* source = "1, 'a'";

    py_Ref r0 = py_getreg(0);
    if(py_eval(source, r0)){
        py_Error* err = py_getlasterror();
        py_Error__print(err);
    }else{
        // handle the result
        py_Ref _0 = py_tuple__getitem(r0, 0);
        py_Ref _1 = py_tuple__getitem(r0, 1);
        int _L0 = py_toint(_0);
        const char* _L1 = py_tostr(_1);
        printf("%d, %s\n", _L0, _L1);
    }

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
