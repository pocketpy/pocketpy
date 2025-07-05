#pragma once

#include "pocketpy/config.h"
#include <stdint.h>

#if PK_ENABLE_OS

typedef void* c11_socket_handler;

enum c11_address_family { C11_AF_INET = 2 };

enum c11_socket_kind { C11_SOCK_STREAM = 1 };

c11_socket_handler c11_socket_create(int family, int type, int protocol);
int c11_socket_bind(c11_socket_handler socket, const char* hostname, unsigned short port);
int c11_socket_listen(c11_socket_handler socket, int backlog);
c11_socket_handler
    c11_socket_accept(c11_socket_handler socket, char* client_ip, unsigned short* client_port);
int c11_socket_connect(c11_socket_handler socket,
                       const char* server_ip,
                       unsigned short server_port);

int c11_socket_recv(c11_socket_handler socket, char* buffer, int maxsize);
int c11_socket_send(c11_socket_handler socket, const char* senddata, int datalen);
int c11_socket_close(c11_socket_handler socket);
int c11_socket_set_block(c11_socket_handler socket, int flag);
c11_socket_handler c11_socket_invalid_socket_handler();
int c11_socket_get_last_error();

#endif // PK_ENABLE_OS