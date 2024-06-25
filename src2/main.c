#include <stdio.h>
#include <stdlib.h>
#include "pocketpy.h"
#include "pocketpy/pocketpy.h"

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

    if(argc != 2) goto __HELP;
    char* source = read_file(argv[1]);
    py_initialize();

    if(py_exec_simple(source)){
        py_Error* err = py_getlasterror();
        py_Error__print(err);
    }
    
    py_finalize();
    free(source);

__HELP:
    printf("Usage: pocketpy [filename]\n");
    return 0;
}
