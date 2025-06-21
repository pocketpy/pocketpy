#pragma once
#include "pocketpy/common/sstream.h"
#include "pocketpy/common/str.h"

typedef void* (*c11_dap_arg_parser_fn)(py_Ref arguments);
typedef bool (*c11_dap_resp_builder_fn)(void* arg);

typedef struct c11_dap_request {
    int seq;
    const char* type;
    const char* command;
    void* arguments;
    c11_dap_resp_builder_fn builder;
} c11_dap_request;

typedef struct c11_dap_initialize_arg {
    int _;
} c11_dap_initialize_arg;

typedef struct c11_dap_attach_arg {
    int _;
} c11_dap_attach_arg;

typedef struct c11_dap_next_arg {
    int _;
} c11_dap_next_arg;

typedef struct c11_dap_stepin_arg {
    int _;
} c11_dap_stepin_arg;

typedef struct c11_dap_stepout_arg {
    int _;
} c11_dap_stepout_arg;

typedef struct c11_dap_threads_arg {
    int _;
} c11_dap_threads_arg;

typedef struct c11_dap_Source {
    const char* name;
} c11_dap_Source;

typedef struct c11_dap_SourceBreakpoint {
    int line;
} c11_dap_SourceBreakpoint;

typedef struct c11_dap_setBreakpoints_arg {
    c11_dap_Source source;
    c11_vector breakpoints;
} c11_dap_setBreakpoints_arg;

typedef struct c11_dap_stackTrace_arg {
    int threadId;
    unsigned levels;
} c11_dap_stackTrace_arg;

typedef struct c11_dap_scopes_arg {
    int frameId;
} c11_dap_scopes_arg;

typedef struct c11_dap_variables_arg {
    int variablesReference;
} c11_dap_variables_arg;

c11_dap_request c11_dap_handle_request(const char* message);
c11_string* c11_dap_handle_response(c11_dap_request request);