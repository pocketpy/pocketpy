#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pocketpy.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static BOOL WINAPI sigint_handler(DWORD dwCtrlType) {
    if(dwCtrlType == CTRL_C_EVENT) {
        py_interrupt();
        return TRUE;
    }
    return FALSE;
}

#else

// set ctrl+c handler
#include <signal.h>
#include <unistd.h>

static void sigint_handler(int sig) { py_interrupt(); }

#endif

char* read_file(const char* path) {
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

static char buf[2048];

int main(int argc, char** argv) {
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigint_handler, TRUE);
#else
    signal(SIGINT, sigint_handler);
#endif

    if(argc > 2) {
        printf("Usage: pocketpy [filename]\n");
        return 0;
    }

    py_initialize();
    py_sys_setargv(argc, argv);

    if(argc == 1) {
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
        char* source = read_file(argv[1]);
        if(source) {
            if(!py_exec(source, argv[1], EXEC_MODE, NULL)) py_printexc();
            free(source);
        }
    }

    int code = py_checkexc(false) ? 1 : 0;
    py_finalize();
    return code;
}
