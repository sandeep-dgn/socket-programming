#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET(s) closesocket(s)
    #define INIT_SOCKETS() WSAStartup(MAKEWORD(2,2), &wsaData)
    #define CLEANUP_SOCKETS() WSACleanup()
    typedef SOCKET socket_t;
    WSADATA wsaData; // Add this line
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSE_SOCKET(s) close(s)
    #define INIT_SOCKETS() 0  // No initialization needed for Linux
    #define CLEANUP_SOCKETS() // No cleanup needed for Linux
    typedef int socket_t;
#endif

// Function prototypes
int start_server(int port);
void handle_client(socket_t client_sock);

#endif // SERVER_H
