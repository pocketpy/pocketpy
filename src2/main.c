#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "pocketpy.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if(file == NULL) {
        printf("Error: file not found\n");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(size + 1);
    size = fread(buffer, 1, size, file);
    buffer[size] = 0;
    return buffer;
}

// void LineProfiler__tracefunc(py_Frame* frame, enum py_TraceEvent event);
static char buf[2048];

int main(int argc, char** argv) {
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    bool profile = false;
    const char* filename = NULL;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--profile") == 0) {
            profile = true;
            continue;
        }
        if(filename == NULL) {
            filename = argv[i];
            continue;
        }
        printf("Usage: pocketpy [--profile] filename\n");
    }

    py_initialize();
    py_sys_setargv(argc, argv);

    assert(!profile);  // not implemented yet
    // if(profile) py_sys_settrace(LineProfiler__tracefunc, true);

    if(filename == NULL) {
        printf("pocketpy " PK_VERSION " (" __DATE__ ", " __TIME__ ") ");
        printf("[%d bit] on %s", (int)(sizeof(void*) * 8), PY_SYS_PLATFORM_STRING);
#ifndef NDEBUG
        printf(" (DEBUG)");
#endif
        printf("\n");
        printf("https://github.com/pocketpy/pocketpy\n");
        printf("Type \"exit()\" to exit.\n");

        while(true) {
            int size = py_replinput(buf, sizeof(buf));
            if(size == -1) {  // Ctrl-D (i.e. EOF)
                printf("\n");
                break;
            }
            assert(size < sizeof(buf));
            if(size >= 0) {
                py_StackRef p0 = py_peek(0);
                if(!py_exec(buf, "<stdin>", SINGLE_MODE, NULL)) {
                    py_printexc();
                    py_clearexc(p0);
                }
            }
        }
    } else {
        char* source = read_file(filename);
        if(source) {
            if(!py_exec(source, filename, EXEC_MODE, NULL)) py_printexc();
            free(source);
        }
    }

    int code = py_checkexc(false) ? 1 : 0;
    py_finalize();
    return code;
}
