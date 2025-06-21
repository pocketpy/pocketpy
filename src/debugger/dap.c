#include <ctype.h>
#include <stdbool.h>
#include "pocketpy/debugger/core.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/debugger/dap.h"


#define DAP_COMMAND_LIST(X)                                                                        \
    X(initialize)                                                                                  \
    X(setBreakpoints)                                                                              \
    X(attach)                                                                                      \
    X(next)                                                                                        \
    X(stepin)                                                                                      \
    X(stepout)                                                                                     \
    X(stackTrace)                                                                                  \
    X(scopes)                                                                                      \
    X(variables)                                                                                   \
    X(threads)

#define DECLARE_ARG_FN(name) void* c11_dap_arg_##name(py_Ref arguments);
DAP_COMMAND_LIST(DECLARE_ARG_FN)
#undef DECLARE_ARG_FN

#define DECLARE_RESP_FN(name) bool c11_dap_body_##name(c11_dap_##name##_arg* arg);
DAP_COMMAND_LIST(DECLARE_RESP_FN)
#undef DECLARE_RESP_FN

#define DEFINE_WRAPPER(name)                                                                       \
    static bool c11_dap_body_##name##_wrapper(void* arg) {                                         \
        return c11_dap_body_##name((c11_dap_##name##_arg*)arg);                                    \
    }
DAP_COMMAND_LIST(DEFINE_WRAPPER)
#undef DEFINE_WRAPPER

typedef struct {
    const char* command;
    c11_dap_arg_parser_fn parser;
    c11_dap_resp_builder_fn builder;
} dap_command_entry;

#define DAP_ENTRY(name) {#name, c11_dap_arg_##name, c11_dap_body_##name##_wrapper},
static dap_command_entry dap_command_table[] = {
    DAP_COMMAND_LIST(DAP_ENTRY){NULL, NULL, NULL}
};
#undef DAP_ENTRY

#undef DAP_COMMAND_LIST



static int dap_next_seq = 1;

void* c11_dap_arg_initialize(py_Ref arguments) { return NULL; };

void* c11_dap_arg_attach(py_Ref arguments) { return NULL; }

void* c11_dap_arg_next(py_Ref arguments) { return NULL; }

void* c11_dap_arg_stepin(py_Ref arguments) { return NULL; }

void* c11_dap_arg_stepout(py_Ref arguments) { return NULL; }

void* c11_dap_arg_threads(py_Ref arguments) { return NULL; }

void* c11_dap_arg_setBreakpoints(py_Ref arguments) {
    c11_dap_setBreakpoints_arg* arg = PK_MALLOC(sizeof(c11_dap_setBreakpoints_arg));
    c11_vector__ctor(&arg->breakpoints, sizeof(c11_dap_SourceBreakpoint));
    py_smarteval("_0['source']['name']", NULL, arguments);
    arg->source.name = c11_strdup(py_tostr(py_retval()));
    py_smarteval("_0['breakpoints']", NULL, arguments);
    for(int i = 0; i < py_list_len(py_retval()); i++) {
        int line = py_toint(py_list_getitem(py_retval(), i));
        c11_vector__push(int, &arg->breakpoints, line);
    }
    return arg;
};

void* c11_dap_arg_stackTrace(py_Ref arguments) {
    c11_dap_stackTrace_arg* arg = PK_MALLOC(sizeof(c11_dap_stackTrace_arg));
    py_smarteval("_0['threadID']", NULL, arguments);
    arg->threadId = py_toint(py_retval());
    py_smarteval("_0.get('levels',-1)", NULL, arguments);
    arg->levels = py_toint(py_retval());
    return arg;
}

void* c11_dap_arg_scopes(py_Ref arguments) {
    c11_dap_scopes_arg* arg = PK_MALLOC(sizeof(c11_dap_scopes_arg));
    py_dict_getitem_by_str(arguments, "frameId");
    arg->frameId = py_toint(py_retval());
    return arg;
}

void* c11_dap_arg_variables(py_Ref arguments) {
    c11_dap_variables_arg* arg = PK_MALLOC(sizeof(c11_dap_variables_arg));
    py_dict_getitem_by_str(arguments, "variablesReference");
    arg->variablesReference = py_toint(py_retval());
    return arg;
}

c11_dap_request c11_dap_handle_request(const char* message) {
    py_Ref py_request = py_pushtmp();
    py_Ref py_arguments = py_pushtmp();
    c11_dap_request request = {.arguments = NULL, .type = "request"};

    if(!py_json_loads(message)) {
        py_printexc();
        exit(0);
    }
    py_assign(py_request, py_retval());

    py_dict_getitem_by_str(py_request, "seq");
    request.seq = py_toint(py_retval());

    py_dict_getitem_by_str(py_request, "command");
    request.command = c11_strdup(py_tostr(py_retval()));

    py_dict_getitem_by_str(py_request, "arguments");
    py_assign(py_arguments, py_retval());

    for(dap_command_entry* entry = dap_command_table; entry->command != NULL; entry++) {
        if(strcmp(entry->command, request.command) == 0) {
            request.arguments = entry->parser(py_arguments);
            request.builder = entry->builder;
            break;
        }
    }

    py_pop();  // py_arguments
    py_pop();  // py_request
    return request;
}

bool c11_dap_body_attach(c11_dap_attach_arg* arg) {
    py_newdict(py_retval());
    return true;
}

bool c11_dap_body_next(c11_dap_next_arg* arg) {
    py_newdict(py_retval());
    return true;
}

bool c11_dap_body_stepin(c11_dap_stepin_arg* arg) {
    py_newdict(py_retval());
    return true;
}

bool c11_dap_body_stepout(c11_dap_stepout_arg* arg) {
    py_newdict(py_retval());
    return true;
}

bool c11_dap_body_initialize(c11_dap_initialize_arg* arg) {
    py_json_loads("{'supportsConfigurationDoneRequest' : true}");
    return true;
}

bool c11_dap_body_setBreakpoints(c11_dap_setBreakpoints_arg* arg) {
    const char* filename = arg->source.name;
    c11__foreach(c11_dap_SourceBreakpoint, &arg->breakpoints, it) {
        c11_debugger_setbreakpoint(filename, it->line);
    }
    py_newdict(py_retval());
    return true;
}

bool c11_dap_build_stackFrame(c11_debugger_frame* frame) {
    c11_sbuf stackFrame_str;
    c11_sbuf__ctor(&stackFrame_str);
    pk_sprintf(
        &stackFrame_str,
        "{\"id\" : %d , \"name\" : \"%s\", \"source\" : {\"name\" : \"%s\", \"path\" : \"%s\"}, \"line\" : %d, \"column\" : %d}",
        frame->id,
        frame->module_name,
        frame->file_name,
        frame->file_name,
        frame->lineno,
        0);
    c11_string* result = c11_sbuf__submit(&stackFrame_str);
    if(!py_json_loads(result->data)) {
        py_printexc();
        c11_string__delete(result);
        return false;
    }
    c11_string__delete(result);
    return true;
}

bool c11_dap_body_stackTrace(c11_dap_stackTrace_arg* arg) {
    py_Ref stackFrames = py_pushtmp();
    c11_vector frames = c11_debugger_get_callstack();

    py_newlist(stackFrames);
    c11__foreach(c11_debugger_frame, &frames, it) {
        c11_dap_build_stackFrame(it);
        py_list_append(stackFrames, py_retval());
    }
    if(!py_smarteval("{'stackFrames' : _0, 'totalFrames' : len(_0)}", NULL, stackFrames)) {
        py_printexc();
        return false;
    }
    py_pop();
    return true;
}

bool c11_dap_build_Scope(c11_debugger_variable* scope) {
    c11_sbuf Scope_str;
    c11_sbuf__ctor(&Scope_str);
    pk_sprintf(&Scope_str,
               "{\"name\" : \"%s\",  \"variablesReference\" : %d, \"expensive\" : false}",
               scope->name,
               scope->id);
    c11_string* result = c11_sbuf__submit(&Scope_str);
    py_json_loads(result->data);
    c11_string__delete(result);
    return true;
}

bool c11_dap_body_scopes(c11_dap_scopes_arg* arg) {
    py_Ref py_scopes = py_pushtmp();
    c11_vector scopes = c11_debugger_get_scope(arg->frameId);
    py_newlist(py_scopes);
    c11__foreach(c11_debugger_variable, &scopes, it) {
        c11_dap_build_Scope(it);
        py_list_append(py_scopes, py_retval());
    }
    py_smarteval("{'scopes' : _0}", NULL, py_scopes);
    py_pop();
    return true;
}

bool c11_dap_build_Variable(c11_debugger_variable* var) {
    c11_sbuf var_str;
    c11_sbuf__ctor(&var_str);
    int variablesReference = var->is_composite ? var->id : 0;
    const char* type = py_tpname(var->type);
    pk_sprintf(
        &var_str,
        "{\"name\" : \"%s\", \"value\" : \"%s\", \"type\" : \"%s\", \"variablesReference\" : %d}",
        var->name,
        var->value,
        type,
        variablesReference);
    c11_string* result = c11_sbuf__submit(&var_str);
    py_json_loads(result->data);
    c11_string__delete(result);
    return true;
}

bool c11_dap_body_variables(c11_dap_variables_arg* arg) {
    py_Ref py_variables = py_pushtmp();
    c11_vector variables = c11_debugger_get_children(arg->variablesReference);
    py_newlist(py_variables);
    c11__foreach(c11_debugger_variable, &variables, it) {
        c11_dap_build_Variable(it);
        py_list_append(py_variables, py_retval());
    }
    py_smarteval("{'variables' : _0}", NULL, py_variables);
    py_pop();
    return true;
}

bool c11_dap_body_threads(c11_dap_threads_arg* arg) {
    py_eval("{'threads' : [{'id' : 1, 'name' : 'Main Thread'}]}", NULL);
    return true;
}

c11_string* c11_dap_handle_response(c11_dap_request request) {
    bool success = request.builder(request.arguments);
    if(!success) {
        printf("An error occured when handle %s request\n", request.command);
        return NULL;
    }

    py_Ref py_body = py_pushtmp();
    c11_sbuf sbuf;
    c11_sbuf__ctor(&sbuf);
    py_assign(py_body, py_retval());
    const char* body_json = py_tostr(py_body);
    if(!py_json_dumps(py_body, 2)){
        py_printexc();
        py_pop();
        c11_sbuf__dtor(&sbuf);
        return NULL;
    }
    pk_sprintf(
        &sbuf,
        "{\"type\":\"response\",\"command\":\"%s\",\"request_seq\":%d,\"success\":%s,\"body\":%s}",
        request.command,
        dap_next_seq++,
        success ? "true" : "false",
        py_tostr(py_retval()));
    py_pop();  // py_body
    return c11_sbuf__submit(&sbuf);
}

