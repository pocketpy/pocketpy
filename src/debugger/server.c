#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pocketpy/common/socket.h"
#include "pocketpy/debugger/dap.h"
#include "pocketpy/pocketpy.h"
#include "pocketpy/debugger/core.h"

static c11_socket_handler g_client_socket = NULL;

static int parse_content_length(const char* buffer) {
    const char* header = strstr(buffer, "Content-Length:");
    if (!header) return -1;
    int length = 0;
    sscanf(header, "Content-Length: %d", &length);
    return length;
}

static const char* find_json_body(const char* buffer) {
    const char* pos = strstr(buffer, "\r\n\r\n");
    return pos ? pos + 4 : NULL;
}

static void send_dap_message(c11_string* dap_response) {
    char header[128];
    sprintf(header, "Content-Length: %d\r\n\r\n", dap_response->size);
    c11_socket_send(g_client_socket, header, strlen(header));
    c11_socket_send(g_client_socket, dap_response->data, dap_response->size);
}


void tracefunc(py_Frame* frame, enum py_TraceEvent event) {
    c11_debugger_on_trace(frame, event);
    while (c11_debugger_should_pause()) {
        char buffer[4096];
        int received = c11_socket_recv(g_client_socket, buffer, sizeof(buffer) - 1);
        if (received <= 0) return;
        buffer[received] = '\0';

        int content_len = parse_content_length(buffer);
        const char* body = find_json_body(buffer);
        if (!body || content_len <= 0) continue;

        c11_dap_request req = c11_dap_handle_request(body);
        c11_string* res = c11_dap_handle_response(req);
        if (res) send_dap_message(res);
    }
}

void c11_dap_server_start(unsigned short port) {
    c11_socket_handler server = c11_socket_create(C11_AF_INET, C11_SOCK_STREAM, 0);
    if (!server) {
        fprintf(stderr, "Failed to create server socket\n");
        return;
    }

    if (c11_socket_bind(server, "127.0.0.1", port) != 0 ||
        c11_socket_listen(server, 1) != 0) {
        fprintf(stderr, "Bind or Listen failed\n");
        c11_socket_close(server);
        return;
    }

    printf("DAP server listening on port %d\n", port);
    char client_ip[64];
    unsigned short client_port;

    g_client_socket = c11_socket_accept(server, client_ip, &client_port);
    if (!g_client_socket) {
        fprintf(stderr, "Accept failed\n");
        c11_socket_close(server);
        return;
    }

    while (1) {
        char buffer[4096];
        int received = c11_socket_recv(g_client_socket, buffer, sizeof(buffer) - 1);
        if (received <= 0) break;
        buffer[received] = '\0';

        int content_len = parse_content_length(buffer);
        const char* body = find_json_body(buffer);
        if (!body || content_len <= 0) continue;

        c11_dap_request req = c11_dap_handle_request(body);
        c11_string* res = c11_dap_handle_response(req);
        if (res) send_dap_message(res);

        if (strcmp(req.command, "configurationDone") == 0) break;
    }

    c11_socket_set_block(g_client_socket, false); // set to not block
    c11_socket_close(server);
    // py_sys_settrace(tracefunc);
}
