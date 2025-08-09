#include "pocketpy/common/socket.h"

#if PK_ENABLE_OS

#include <stddef.h>

#if defined (_WIN32) || defined (_WIN64)
#include <WinSock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_fd;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
typedef int socket_fd;
#endif

#define SOCKET_HANDLERTOFD(handler) (socket_fd)(uintptr_t)(handler)
#define SOCKET_FDTOHANDLER(fd) (c11_socket_handler)(uintptr_t)(fd)


static int c11_socket_init(){
    #if defined (_WIN32) || defined (_WIN64)
        WORD sockVersion = MAKEWORD(2,2);
	    WSADATA wsaData;
        return WSAStartup(sockVersion, &wsaData); 
    #endif 
    return 0;
}

c11_socket_handler c11_socket_create(int family, int type, int protocol){
    static int is_initialized = 0;
    if(is_initialized == 0)
    {
        c11_socket_init();
        is_initialized = 1;     
    }
    return SOCKET_FDTOHANDLER(socket(family, type, protocol));
}

int c11_socket_bind(c11_socket_handler socket, const char* hostname, unsigned short port){
    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    inet_pton(AF_INET,hostname,&bind_addr.sin_addr);
    if(bind(SOCKET_HANDLERTOFD(socket), (const struct sockaddr*)&bind_addr, sizeof(bind_addr)) == -1){
        return -1;
    }
    return 0;
}

int c11_socket_listen(c11_socket_handler socket, int backlog){
    listen(SOCKET_HANDLERTOFD(socket), backlog);
    return 0;
}

c11_socket_handler c11_socket_accept(c11_socket_handler socket, char* client_ip, unsigned short* client_port){
    struct sockaddr_in client_addr;
    socklen_t sockaddr_len = sizeof(client_addr);
    socket_fd client_socket = accept(SOCKET_HANDLERTOFD(socket), (struct sockaddr*)&client_addr, &sockaddr_len);
    if(client_ip != NULL){
        inet_ntop(AF_INET, &client_addr.sin_addr,client_ip,sizeof("255.255.255.255"));
    }
    if(client_port != NULL){
        *client_port = ntohs(client_addr.sin_port);
    }
    return SOCKET_FDTOHANDLER(client_socket);
}
int c11_socket_connect(c11_socket_handler socket, const char* server_ip, unsigned short server_port){
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    if(connect(SOCKET_HANDLERTOFD(socket), (const struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        return -1;
    }
    return 0;
}

int c11_socket_recv(c11_socket_handler socket, char* buffer, int maxsize){
    return recv(SOCKET_HANDLERTOFD(socket), buffer, maxsize,0);
}

int c11_socket_send(c11_socket_handler socket, const char* senddata, int datalen){
    return send(SOCKET_HANDLERTOFD(socket), senddata, datalen, 0);
}

int c11_socket_close(c11_socket_handler socket){
    #if defined (_WIN32) || defined (_WIN64)
        return closesocket(SOCKET_HANDLERTOFD(socket));
    #else
        return close(SOCKET_HANDLERTOFD(socket));
    #endif
}

int c11_socket_set_block(c11_socket_handler socket,int flag){
    #if defined (_WIN32) || defined (_WIN64)
        u_long mode = flag == 1 ? 0 : 1;
        return ioctlsocket(SOCKET_HANDLERTOFD(socket), FIONBIO, &mode);
    #else
        int flags = fcntl(SOCKET_HANDLERTOFD(socket), F_GETFL, 0);
        return fcntl(SOCKET_HANDLERTOFD(socket), F_SETFL, flags | O_NONBLOCK);
    #endif
}

c11_socket_handler c11_socket_invalid_socket_handler(){
    return (void*)(uintptr_t)(-1);
}


int c11_socket_get_last_error(){
    #if defined (_WIN32) || defined (_WIN64)
        return WSAGetLastError();
    #else
        return errno;
    #endif
}

#undef SOCKET_HANDLERTOFD
#undef SOCKET_FDTOHANDLER

#endif // PK_ENABLE_OS