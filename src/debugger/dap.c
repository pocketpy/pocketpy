#include <stdbool.h>
#include "pocketpy/common/socket.h"
#include "pocketpy/debugger/core.h"
#include "pocketpy/objects/base.h"

#define DAP_COMMAND_LIST(X)                                                                        \
    X(initialize)                                                                                  \
    X(setBreakpoints)                                                                              \
    X(attach)                                                                                      \
    X(next)                                                                                        \
    X(stepIn)                                                                                      \
    X(stepOut)                                                                                     \
    X(continue)                                                                                    \
    X(stackTrace)                                                                                  \
    X(scopes)                                                                                      \
    X(variables)                                                                                   \
    X(threads)                                                                                     \
    X(configurationDone)

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

// #undef DAP_COMMAND_LIST

// static int dap_next_seq = 1;
static struct c11_dap_server {
    int dap_next_seq;
    char buffer_data[1024];
    char* buffer_begin;
    int buffer_length;
    c11_socket_handler server;
    c11_socket_handler toclient;
    bool isconfiguredone;
    bool isatttach;
} server;

void c11_dap_handle_initialize(py_Ref arguments, c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer, "\"body\":{\"supportsConfigurationDoneRequest\":true}");
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_attach(py_Ref arguments, c11_sbuf* buffer) {
    server.isatttach = true;

}

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
    const char* sourcename = py_tostr(py_retval());
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
}

void c11_dap_handle_stackTrace(py_Ref arguments, c11_sbuf* buffer) {
    c11_sbuf__write_cstr(buffer, "\"body\":");
    c11_debugger_frames(buffer);
    c11_sbuf__write_char(buffer, ',');
}

void c11_dap_handle_scopes(py_Ref arguments, c11_sbuf* buffer) {
    int res = py_dict_getitem_by_str(arguments, "frameId");
    if(res <= 0) {
        if(res == 0) {
            printf("[DEBUGGER ERROR] no frameID found\n");
        } else {
            py_printexc();
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
        return NULL;
    }
    py_Ref py_request = py_pushtmp();
    py_Ref py_arguments = py_pushtmp();
    py_Ref py_command = py_pushtmp();
    py_assign(py_request, py_retval());

    int res = py_dict_getitem_by_str(py_request, "command");
    if(res == -1) {
        py_printexc();
        return NULL;
    } else if(res == 0) {
        return "cannot find attribute command";
    }
    py_assign(py_command, py_retval());
    const char* command = py_tostr(py_command);

    res = py_dict_getitem_by_str(py_request, "arguments");
    if(res == -1) {
        py_printexc();
        return NULL;
    }
    py_assign(py_arguments, py_retval());

    res = py_dict_getitem_by_str(py_request, "seq");
    if(res == -1) {
        py_printexc();
        return NULL;
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
    char json[256];
    int json_len = snprintf(json,
                            sizeof(json),
                            "{\"seq\":%d,\"type\":\"event\",\"event\":\"%s\",\"body\":%s}",
                            server.dap_next_seq++,
                            event_name,
                            body_json);

    char header[64];
    int header_len = snprintf(header, sizeof(header), "Content-Length: %d\r\n\r\n", json_len);

    c11_socket_send(server.toclient, header, header_len);
    c11_socket_send(server.toclient, json, json_len);
}

void c11_dap_send_stop_event() {
    c11_dap_send_event("stopped",
                       "{\"reason\":\"breakpoint\",\"threadId\":1,\"allThreadsStopped\":true}");
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
        printf("[DEBUGGER EORRO] : the number is empty\n");
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
    server.buffer_begin = server.buffer_data;
    server.server = c11_socket_create(C11_AF_INET, C11_SOCK_STREAM, 0);
    c11_socket_bind(server.server, hostname, port);
    c11_socket_listen(server.server, 0);
    printf("[DEBUGGER INFO] : listen on %s:%hu\n", hostname, port);
    server.toclient = c11_socket_accept(server.server, NULL, NULL);
    printf("[DEBUGGER INFO] : connected client\n");
}

inline static void c11_dap_handle_message() {
    const char* message = c11_dap_read_message();
    if(message == NULL) return;
    printf("[DEBUGGER INFO] read request %s\n", message);
    const char* response_content = c11_dap_handle_request(message);
    if(response_content != NULL) { printf("[DEBUGGER INFO] send response %s\n", response_content); }
    c11_sbuf buffer;
    c11_sbuf__ctor(&buffer);
    pk_sprintf(&buffer, "Content-Length: %d\r\n\r\n%s", strlen(response_content), response_content);
    c11_string* response = c11_sbuf__submit(&buffer);
    c11_socket_send(server.toclient, response->data, response->size);
    free((void*)message);
    free((void*)response_content);
    c11_string__delete(response);
}

void c11_dap_configure_debugger() {
    while(server.isconfiguredone == false) {
        c11_dap_handle_message();
        if(server.isatttach) {
            c11_dap_send_initialized_event();
            server.isatttach = false;
        }
    }
    printf("[DEBUGGER INFO] : configure done\n");
}

void c11_dap_tracefunc(py_Frame* frame, enum py_TraceEvent event) {
    py_sys_settrace(NULL, false);
    c11_debugger_on_trace(frame, event);
    c11_dap_handle_message();
    if(!c11_debugger_should_pause()) {
        py_sys_settrace(c11_dap_tracefunc, false);
        return;
    }
    c11_dap_send_stop_event();
    while(c11_debugger_should_keep_pause()) {
        c11_dap_handle_message();
    }
    py_sys_settrace(c11_dap_tracefunc, false);
}

void py_debugger_waitforattach(const char* hostname, unsigned short port) {
    c11_dap_init_server(hostname, port);
    c11_debugger_init();
    c11_dap_configure_debugger();
    c11_socket_set_block(server.toclient, 0);
    py_sys_settrace(c11_dap_tracefunc, true);
}

void py_debugger_exit(int exitCode) {
    char body[64];
    snprintf(body, sizeof(body), "{\"exitCode\":%d}", exitCode);
    c11_dap_send_event("exited", body);
}