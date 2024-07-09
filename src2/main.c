#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pocketpy.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

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

static char buf[2048];

int main(int argc, char** argv) {
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    if(argc > 2) {
        printf("Usage: pocketpy [filename]\n");
        return 0;
    }

    py_initialize();

    if(argc == 1) {
        printf("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ") ");
        printf("[%d bit] on %s\n", (int)(sizeof(void*) * 8), PY_SYS_PLATFORM_STRING);
        printf("https://github.com/pocketpy/pocketpy\n");
        printf("Type \"exit()\" to exit.\n");

        while(true) {
            int size = py_replinput(buf);
            assert(size < sizeof(buf));
            if(size >= 0) {
                if(!py_exec2(buf, "<stdin>", REPL_MODE)) py_printexc();
            }
        }
    } else {
        char* source = read_file(argv[1]);
        if(source) {
            if(!py_exec(source)) py_printexc();
            free(source);
        }
    }

    int code = py_checkexc() ? 1 : 0;
    py_finalize();
    return code;
}
