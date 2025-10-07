#include <stdbool.h>
#include <stdio.h>
#include "pocketpy/common/socket.h"
#include "pocketpy/debugger/core.h"
#include "pocketpy/objects/base.h"
#include "pocketpy/objects/exception.h"
#include "pocketpy/pocketpy.h"

#if PK_ENABLE_OS

#define DAP_COMMAND_LIST(X)                                                                        \
    X(initialize)                                                                                  \
    X(setBreakpoints)                                                                              \
    X(attach)                                                                                      \
    X(launch)                                                                                      \
    X(next)                                                                                        \
    X(stepIn)                                                                                      \
    X(stepOut)                                                                                     \
    X(continue)                                                                                    \
    X(stackTrace)                                                                                  \
    X(scopes)                                                                                      \
    X(variables)                                                                                   \
    X(threads)                                                                                     \
    X(configurationDone)                                                                           \
    X(ready)                                                                                       \
    X(evaluate)                                                                                    \
    X(exceptionInfo)

#define DECLARE_HANDLE_FN(name) void c11_dap_handle_##name(py_Ref arguments, c11_sbuf*);
DAP_COMMAND_LIST(DECLARE_HANDLE_FN)
#undef DECLARE_ARG_FN

typedef void (*c11_dap_arg_parser_fn)(py_Ref, c11_sbuf*);

typedef struct {
    const char* command;
    c11_dap_arg_parser_fn parser;
} dap_command_entry;

#define DAP_ENTRY(name) {#name, c11_dap_handle_##name},
static dap_command_entry dap_command_table[] = {
    DAP_COMMAND_LIST(DAP_ENTRY){NULL, NULL}
};

#undef DAP_ENTRY

static struct c11_dap_server {
    int dap_next_seq;
    char buffer_data[1024];
    char* buffer_begin;
    int buffer_length;
    c11_socket_handler server;
    c11_socket_handler toclient;
    bool isconfiguredone;
    bool isfirstatttach;
    bool isUserCode;
    bool isAttached;
    bool isclientready;
} server;

void c11_dap_handle_initialize(py_Ref arguments, c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer,
                         "\"body\":{"
                         "\"supportsConfigurationDoneRequest\":true,"
                         "\"supportsExceptionInfoRequest\":true"
                         "}");
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_attach(py_Ref arguments, c11_sbuf* buffer) { server.isfirstatttach = true; }

void c11_dap_handle_launch(py_Ref arguments, c11_sbuf* buffer) { server.isfirstatttach = true; }

void c11_dap_handle_ready(py_Ref arguments, c11_sbuf* buffer) { server.isclientready = true; }

void c11_dap_handle_next(py_Ref arguments, c11_sbuf* buffer) {
    c11_debugger_set_step_mode(C11_STEP_OVER);
}

void c11_dap_handle_stepIn(py_Ref arguments, c11_sbuf* buffer) {
    c11_debugger_set_step_mode(C11_STEP_IN);
}

void c11_dap_handle_stepOut(py_Ref arguments, c11_sbuf* buffer) {
    c11_debugger_set_step_mode(C11_STEP_OUT);
}

void c11_dap_handle_continue(py_Ref arguments, c11_sbuf* buffer) {
    c11_debugger_set_step_mode(C11_STEP_CONTINUE);
}

void c11_dap_handle_threads(py_Ref arguments, c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer,
                         "\"body\":{\"threads\":["
                         "{\"id\":1,\"name\":\"MainThread\"}"
                         "]}");
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_configurationDone(py_Ref arguments, c11_sbuf* buffer) {
    server.isconfiguredone = true;
}

inline static void c11_dap_build_Breakpoint(int id, int line, c11_sbuf* buffer) {
    pk_sprintf(buffer, "{\"id\":%d,\"verified\":true,\"line\":%d}", id, line);
}

void c11_dap_handle_setBreakpoints(py_Ref arguments, c11_sbuf* buffer) {
    if(!py_smarteval("_0['source']['path']", NULL, arguments)) {
        py_printexc();
        return;
    }
    const char* sourcename = c11_strdup(py_tostr(py_retval()));
    if(!py_smarteval("[bp['line'] for bp in _0['breakpoints']]", NULL, arguments)) {
        py_printexc();
        return;
    }
    int bp_numbers = c11_debugger_reset_breakpoints_by_source(sourcename);
    c11_sbuf__write_cstr(buffer, "\"body\":");
    c11_sbuf__write_cstr(buffer, "{\"breakpoints\":[");
    for(int i = 0; i < py_list_len(py_retval()); i++) {
        if(i != 0) c11_sbuf__write_char(buffer, ',');
        int line = py_toint(py_list_getitem(py_retval(), i));
        c11_debugger_setbreakpoint(sourcename, line);
        c11_dap_build_Breakpoint(i + bp_numbers, line, buffer);
    }
    c11_sbuf__write_cstr(buffer, "]}");
    c11_sbuf__write_char(buffer, ',');
    PK_FREE((void*)sourcename);
}

inline static void c11_dap_build_ExceptionInfo(const char* exc_type, const char* exc_message, c11_sbuf* buffer) {
    const char* safe_type = exc_type ? exc_type : "UnknownException";
    const char* safe_message = exc_message ? exc_message : "No additional details available";

    c11_sv type_sv = {.data = safe_type, .size = strlen(safe_type)};
    c11_sv message_sv = {.data = safe_message, .size = strlen(safe_message)};

    c11_sbuf combined_buffer;
    c11_sbuf__ctor(&combined_buffer);
    pk_sprintf(&combined_buffer, "%s: %s", safe_type, safe_message);
    c11_string* combined_details = c11_sbuf__submit(&combined_buffer);

    pk_sprintf(buffer, 
        "{\"exceptionId\":%Q,"
        "\"description\":%Q,"
        "\"breakMode\":\"unhandled\"}",
        type_sv,
        (c11_sv){.data = combined_details->data, .size = combined_details->size}
    );

    c11_string__delete(combined_details);
}

void c11_dap_handle_exceptionInfo(py_Ref arguments, c11_sbuf* buffer) {
    const char* message;
    const char* name = c11_debugger_excinfo(&message);
    c11_sbuf__write_cstr(buffer, "\"body\":");
    c11_dap_build_ExceptionInfo(name, message, buffer);
    c11_sbuf__write_char(buffer, ',');
}


void c11_dap_handle_stackTrace(py_Ref arguments, c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer, "\"body\":");
    c11_debugger_frames(buffer);
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_evaluate(py_Ref arguments, c11_sbuf* buffer) {
    int res = py_dict_getitem_by_str(arguments, "expression");
    if(res <= 0) {
        py_printexc();
        c11__abort("[DEBUGGER ERROR] no expression found in evaluate request");
    }
    // [eval, nil, expression,  globals, locals]
    // vectorcall would pop the above 5 items
    // so we don't need to pop them manually
    py_StackRef p0 = py_peek(0);
    py_Ref py_eval = py_pushtmp();
    py_pushnil();
    py_Ref expression = py_pushtmp();
    py_assign(expression, py_retval());
    py_assign(py_eval, py_getbuiltin(py_name("eval")));
    py_newglobals(py_pushtmp());
    py_newlocals(py_pushtmp());
    bool ok = py_vectorcall(3, 0);

    char* result = NULL;
    c11_sbuf__write_cstr(buffer, "\"body\":");
    if(!ok) {
        result = py_formatexc();
        py_clearexc(p0);
    } else {
        py_Ref py_result = py_pushtmp();
        py_assign(py_result, py_retval());
        py_str(py_result);
        py_assign(py_result, py_retval());
        result = c11_strdup(py_tostr(py_result));
        py_pop();
    }

    c11_sv result_sv = {.data = result, .size = strlen(result)};
    pk_sprintf(buffer, "{\"result\":%Q,\"variablesReference\":0}", result_sv);
    PK_FREE((void*)result);
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_scopes(py_Ref arguments, c11_sbuf* buffer) {
    int res = py_dict_getitem_by_str(arguments, "frameId");
    if(res <= 0) {
        if(res == 0) {
            c11__abort("[DEBUGGER ERROR] no frameID found in scopes request");
        } else {
            py_printexc();
            c11__abort("[DEBUGGER ERROR] an error occurred while parsing request frameId");
        }
        return;
    }
    int frameid = py_toint(py_retval());
    c11_sbuf__write_cstr(buffer, "\"body\":");
    c11_debugger_scopes(frameid, buffer);
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_variables(py_Ref arguments, c11_sbuf* buffer) {
    int res = py_dict_getitem_by_str(arguments, "variablesReference");
    if(res <= 0) {
        if(res == 0) {
            printf("[DEBUGGER ERROR] no frameID found\n");
        } else {
            py_printexc();
        }
        return;
    }
    int variablesReference = py_toint(py_retval());
    c11_sbuf__write_cstr(buffer, "\"body\":");
    c11_debugger_unfold_var(variablesReference, buffer);
    c11_sbuf__write_char(buffer, ',');
}

const char* c11_dap_handle_request(const char* message) {
    if(!py_json_loads(message)) {
        py_printexc();
        c11__abort("[DEBUGGER ERROR] invalid JSON request");
    }
    py_Ref py_request = py_pushtmp();
    py_Ref py_arguments = py_pushtmp();
    py_Ref py_command = py_pushtmp();
    py_assign(py_request, py_retval());

    int res = py_dict_getitem_by_str(py_request, "command");
    if(res == -1) {
        py_printexc();
        c11__abort("[DEBUGGER ERROR] an error occurred while parsing request");
    } else if(res == 0) {
        c11__abort("[DEBUGGER ERROR] no command found in request");
    }
    py_assign(py_command, py_retval());
    const char* command = py_tostr(py_command);

    res = py_dict_getitem_by_str(py_request, "arguments");
    if(res == -1) {
        py_printexc();
        c11__abort("[DEBUGGER ERROR] an error occurred while parsing request arguments");
    }
    py_assign(py_arguments, py_retval());

    res = py_dict_getitem_by_str(py_request, "seq");
    if(res == -1) {
        py_printexc();
        c11__abort("[DEBUGGER ERROR] an error occurred while parsing request sequence number");
    }
    int request_seq = (res == 1) ? py_toint(py_retval()) : 0;

    c11_sbuf response_buffer;
    c11_sbuf__ctor(&response_buffer);
    pk_sprintf(&response_buffer,
               "{\"seq\":%d,\"type\":\"response\",\"request_seq\":%d,\"command\":\"%s\",",
               server.dap_next_seq++,
               request_seq,
               command);
    for(dap_command_entry* entry = dap_command_table; entry->command != NULL; entry++) {
        if(strcmp(entry->command, command) == 0) {
            entry->parser(py_arguments, &response_buffer);
            break;
        }
    }
    c11_sbuf__write_cstr(&response_buffer, "\"success\":true}");

    c11_string* c11_string_response = c11_sbuf__submit(&response_buffer);
    const char* response = c11_strdup(c11_string_response->data);
    c11_string__delete(c11_string_response);

    py_pop();  // py_arguments
    py_pop();  // py_request
    py_pop();  // py_command

    return response;
}

void c11_dap_send_event(const char* event_name, const char* body_json) {
    c11_sbuf buffer;
    char header[64];
    c11_sbuf__ctor(&buffer);
    pk_sprintf(&buffer,
               "{\"seq\":%d,\"type\":\"event\",\"event\":\"%s\",\"body\":%s}",
               server.dap_next_seq++,
               event_name,
               body_json);
    c11_string* json = c11_sbuf__submit(&buffer);
    int json_len = json->size;
    int header_len = snprintf(header, sizeof(header), "Content-Length: %d\r\n\r\n", json_len);
    c11_socket_send(server.toclient, header, header_len);
    c11_socket_send(server.toclient, json->data, json_len);
    c11_string__delete(json);
}

void c11_dap_send_output_event(const char* category, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    c11_sbuf output;
    c11_sbuf__ctor(&output);
    pk_vsprintf(&output, fmt, args);
    va_end(args);

    c11_sbuf buffer;
    c11_string* output_json = c11_sbuf__submit(&output);
    c11_sv sv_output = {.data = output_json->data, .size = output_json->size};
    c11_sbuf__ctor(&buffer);
    pk_sprintf(&buffer, "{\"category\":\"%s\",\"output\":%Q}", category, sv_output);
    c11_string* body_json = c11_sbuf__submit(&buffer);
    c11_dap_send_event("output", body_json->data);
    c11_string__delete(body_json);
    c11_string__delete(output_json);
}

void c11_dap_send_stop_event(C11_STOP_REASON reason) {
    if(reason == C11_DEBUGGER_NOSTOP) return;
    const char* reason_str = "unknown";
    switch(reason) {
        case C11_DEBUGGER_STEP: reason_str = "step"; break;
        case C11_DEBUGGER_EXCEPTION: reason_str = "exception"; break;
        case C11_DEBUGGER_BP: reason_str = "breakpoint"; break;
        default: break;
    }
    c11_sbuf buf;
    c11_sbuf__ctor(&buf);
    pk_sprintf(&buf, "{\"reason\":\"%s\",\"threadId\":1,\"allThreadsStopped\":true", reason_str);
    c11_sbuf__write_char(&buf, '}');
    c11_string* body_json = c11_sbuf__submit(&buf);
    c11_dap_send_event("stopped", body_json->data);
    c11_string__delete(body_json);
}

void c11_dap_send_exited_event(int exitCode) {
    char body[64];
    snprintf(body, sizeof(body), "{\"exitCode\":%d}", exitCode);
    c11_dap_send_event("exited", body);
}

void c11_dap_send_fatal_event(const char* message) {
    char body[128];
    snprintf(body, sizeof(body), "{\"message\":\"%s\"}", message);
    c11_dap_send_event("pkpy/fatalError", body);
}

void c11_dap_send_initialized_event() { c11_dap_send_event("initialized", "{}"); }

int c11_dap_read_content_length(const char* buffer, int* header_length) {
    const char* length_begin = strstr(buffer, "Content-Length: ");
    if(!length_begin) {
        printf("[DEBUGGER ERROR] : no Content-Length filed found\n");
        *header_length = 0;
        return -1;
    }
    length_begin += strlen("Content-Length: ");
    const char* length_end = strstr(length_begin, "\r\n\r\n");
    if(!length_end) {
        printf("[DEBUGGER ERROR] : the seperator should br \\r\\n\\r\\n\n");
        *header_length = 0;
        return -1;
    }
    char* endptr = NULL;
    long value = strtol(length_begin, &endptr, 10);
    if(endptr == length_begin) {
        printf("[DEBUGGER EORRO] : the length field is empty\n");
        *header_length = 0;
        return -1;
    }
    *header_length = (int)(endptr - buffer) + 4;
    return (int)value;
}

const char* c11_dap_read_message() {
    int message_length =
        c11_socket_recv(server.toclient, server.buffer_begin, 1024 - server.buffer_length);
    if(message_length == 0) {
        printf("[DEBUGGER INFO] : client quit\n");
        exit(0);
    }
    if(message_length < 0) { return NULL; }
    server.buffer_length += message_length;
    if(server.buffer_length == 0) return NULL;
    int header_length;
    int content_length = c11_dap_read_content_length(server.buffer_begin, &header_length);
    if(content_length <= 0 || header_length <= 0) {
        printf("[DEBUGGER ERROR]: invalid DAP header\n");
        server.buffer_length = 0;
        server.buffer_begin = server.buffer_data;
        return NULL;
    }
    server.buffer_begin += header_length;
    server.buffer_length -= header_length;
    c11_sbuf result;
    c11_sbuf__ctor(&result);
    while(content_length > server.buffer_length) {
        c11_sbuf__write_cstrn(&result, server.buffer_begin, server.buffer_length);
        content_length -= server.buffer_length;
        message_length = c11_socket_recv(server.toclient, server.buffer_data, 1024);
        if(message_length == 0) {
            printf("[DEBUGGER INFO] : client quit\n");
            exit(0);
        }
        if(message_length < 0) continue;
        server.buffer_begin = server.buffer_data;
        server.buffer_length = message_length;
    }
    c11_sbuf__write_cstrn(&result, server.buffer_begin, content_length);
    server.buffer_begin += content_length;
    server.buffer_length -= content_length;
    memmove(server.buffer_data, server.buffer_begin, server.buffer_length);
    server.buffer_begin = server.buffer_data;
    c11_string* tmp_result = c11_sbuf__submit(&result);
    const char* dap_message = c11_strdup(tmp_result->data);
    c11_string__delete(tmp_result);
    return dap_message;
}

void c11_dap_init_server(const char* hostname, unsigned short port) {
    server.dap_next_seq = 1;
    server.isconfiguredone = false;
    server.isclientready = false;
    server.isfirstatttach = false;
    server.isUserCode = false;
    server.isAttached = false;
    server.buffer_begin = server.buffer_data;
    server.server = c11_socket_create(C11_AF_INET, C11_SOCK_STREAM, 0);
    c11_socket_bind(server.server, hostname, port);
    c11_socket_listen(server.server, 0);
    // c11_dap_send_output_event("console", "[DEBUGGER INFO] : listen on %s:%hu\n",hostname,port);
    printf("[DEBUGGER INFO] : listen on %s:%hu\n", hostname, port);
}

void c11_dap_waitforclient(const char* hostname, unsigned short port) {
    server.toclient = c11_socket_accept(server.server, NULL, NULL);
}

inline static void c11_dap_handle_message() {
    const char* message = c11_dap_read_message();
    if(message == NULL) { return; }
    // c11_dap_send_output_event("console", "[DEBUGGER LOG] : read request %s\n", message);
    const char* response_content = c11_dap_handle_request(message);
    if(response_content != NULL) {
        // c11_dap_send_output_event("console",
        //                           "[DEBUGGER LOG] : send response %s\n",
        //                           response_content);
    }
    c11_sbuf buffer;
    c11_sbuf__ctor(&buffer);
    pk_sprintf(&buffer, "Content-Length: %d\r\n\r\n%s", strlen(response_content), response_content);
    c11_string* response = c11_sbuf__submit(&buffer);
    c11_socket_send(server.toclient, response->data, response->size);
    PK_FREE((void*)message);
    PK_FREE((void*)response_content);
    c11_string__delete(response);
}

void c11_dap_configure_debugger() {
    while(server.isconfiguredone == false) {
        c11_dap_handle_message();
        if(server.isfirstatttach) {
            c11_dap_send_initialized_event();
            server.isfirstatttach = false;
            server.isAttached = true;
            server.isUserCode = true;
        } else if(server.isclientready) {
            server.isclientready = false;
            return;
        }
    }
    // c11_dap_send_output_event("console", "[DEBUGGER INFO] : client configure done\n");
}

void c11_dap_tracefunc(py_Frame* frame, enum py_TraceEvent event) {
    py_sys_settrace(NULL, false);
    server.isUserCode = false;
    C11_DEBUGGER_STATUS result = c11_debugger_on_trace(frame, event);
    if(result == C11_DEBUGGER_EXIT) {
        // c11_dap_send_output_event("console", "[DEBUGGER INFO] : program exit\n");
        c11_dap_send_exited_event(0);
        exit(0);
    }
    if(result != C11_DEBUGGER_SUCCESS) {
        const char* message = NULL;
        switch(result) {
            case C11_DEBUGGER_FILEPATH_ERROR:
                message = "Invalid py_file path: '..' forbidden, './' only allowed at start.";
                break;
            case C11_DEBUGGER_UNKNOW_ERROR:
            default: message = "Unknown debugger failure."; break;
        }
        if(message) { c11_dap_send_fatal_event(message); }
        c11_dap_send_exited_event(1);
        exit(1);
    }
    c11_dap_handle_message();
    C11_STOP_REASON reason = c11_debugger_should_pause();
    if(reason == C11_DEBUGGER_NOSTOP) {
        py_sys_settrace(c11_dap_tracefunc, false);
        server.isUserCode = true;
        return;
    }
    c11_dap_send_stop_event(reason);
    while(c11_debugger_should_keep_pause()) {
        c11_dap_handle_message();
    }
    server.isUserCode = true;
    py_sys_settrace(c11_dap_tracefunc, false);
}

void py_debugger_waitforattach(const char* hostname, unsigned short port) {
    c11_debugger_init();
    c11_dap_init_server(hostname, port);
    while(!server.isconfiguredone) {
        c11_dap_waitforclient(hostname, port);
        c11_dap_configure_debugger();
        if(!server.isconfiguredone) {
            c11_socket_close(server.toclient);
            // c11_dap_send_output_event("console", "[DEBUGGER INFO] : An clinet is ready\n");
        }
    }
    c11_socket_set_block(server.toclient, 0);
    py_sys_settrace(c11_dap_tracefunc, true);
}

void py_debugger_exit(int exitCode) { c11_dap_send_exited_event(exitCode); }

int py_debugger_status() { 
    if(!server.isAttached) {
        return 0;
    }
    return server.isUserCode ? 1 : 2;
 }

void py_debugger_exceptionbreakpoint(py_Ref exc) {
    assert(py_isinstance(exc, tp_BaseException));

    py_sys_settrace(NULL, true);
    server.isUserCode = false;

    c11_debugger_exception_on_trace(exc);
    for(;;) {
        C11_STOP_REASON reason = c11_debugger_should_pause();
        c11_dap_send_stop_event(reason);
        while(c11_debugger_should_keep_pause()) {
            c11_dap_handle_message();
        }
    }
}

#endif // PK_ENABLE_OS