#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "pocketpy.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static char buf[2048];

int main(int argc, char** argv) {
#if _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    bool profile = false;
    bool debug = false;
    bool compile = false;
    const char* arg1 = NULL;
    const char* arg2 = NULL;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--profile") == 0) {
            profile = true;
            continue;
        }
        if(strcmp(argv[i], "--debug") == 0) {
            debug = true;
            continue;
        }
        if(strcmp(argv[i], "--compile") == 0) {
            compile = true;
            continue;
        }
        if(arg1 == NULL) {
            arg1 = argv[i];
            continue;
        }
        if(arg2 == NULL) {
            arg2 = argv[i];
            continue;
        }
        printf("Usage: pocketpy [--profile] [--debug] [--compile] filename\n");
    }

    if(debug && profile) {
        printf("Error: --debug and --profile cannot be used together.\n");
        return 1;
    }

    if(compile && (debug || profile)) {
        printf("Error: --compile cannot be used with --debug or --profile.\n");
        return 1;
    }

    py_initialize();
    py_sys_setargv(argc, argv);

    if(compile) {
        bool ok = py_compilefile(arg1, arg2);
        if(!ok) py_printexc();
        py_finalize();
        return ok ? 0 : 1;
    }

    const char* filename = arg1;
    if(filename == NULL) {
        if(profile) printf("Warning: --profile is ignored in REPL mode.\n");
        if(debug) printf("Warning: --debug is ignored in REPL mode.\n");

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
        if(profile) py_profiler_begin();
        if(debug) py_debugger_waitforattach("127.0.0.1", 6110);

        int data_size;
        char* data = py_callbacks()->importfile(filename, &data_size);
        // check filename endswith .pyc
        bool is_pyc = false;
        int filename_len = (int)strlen(filename);
        if(filename_len >= 4) {
            if(filename[filename_len - 4] == '.' &&
               filename[filename_len - 3] == 'p' &&
               filename[filename_len - 2] == 'y' &&
               filename[filename_len - 1] == 'c') {
                is_pyc = true;
            }
        }

        if(data) {
            bool ok;
            if(is_pyc) {
                ok = py_execo(data, data_size, filename, NULL);
            } else {
                ok = py_exec(data, filename, EXEC_MODE, NULL);
            }
            if(!ok) py_printexc();

            if(profile) {
                char* json_report = py_profiler_report();
                FILE* report_file = fopen("profiler_report.json", "w");
                if(report_file) {
                    fprintf(report_file, "%s", json_report);
                    fclose(report_file);
                }
                PK_FREE(json_report);
            }
            PK_FREE(data);
        } else {
            printf("Error: cannot open file '%s'\n", filename);
            py_finalize();
            return 1;
        }
    }

    int code = py_checkexc() ? 1 : 0;
    py_finalize();

    if(debug) py_debugger_exit(code);
    return code;
}
