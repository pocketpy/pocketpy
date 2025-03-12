#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "pocketpy.h"

#define py_interrupt()

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

static void tracefunc(py_Frame* frame, enum py_TraceEvent event) {
    int line;
    const char* filename = py_Frame_sourceloc(frame, &line);
    const char* event_str;
    switch(event) {
        case TRACE_EVENT_LINE:
            event_str = "line";
            break;
        case TRACE_EVENT_EXCEPTION:
            event_str = "exception";
            break;
        case TRACE_EVENT_PUSH:
            event_str = "push";
            break;
        case TRACE_EVENT_POP:
            event_str = "pop";
            break;
    }
    printf("\x1b[30m%s:%d, event=%s\x1b[0m\n", filename, line, event_str);
}

static char buf[2048];

int main(int argc, char** argv) {
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    // SetConsoleCtrlHandler((PHANDLER_ROUTINE)sigint_handler, TRUE);
#else
    // signal(SIGINT, sigint_handler);
#endif

    bool trace = false;
    const char* filename = NULL;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--trace") == 0) {
            trace = true;
            continue;
        }
        if(filename == NULL) {
            filename = argv[i];
            continue;
        }
        printf("Usage: pocketpy [--trace] filename\n");
    }

    py_initialize();
    py_sys_setargv(argc, argv);

    if(trace) py_sys_settrace(tracefunc);

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
