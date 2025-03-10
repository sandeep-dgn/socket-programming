#ifndef CLIENT_H
#define CLIENT_H

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
    #define INIT_SOCKETS() 0  // No need for WSAStartup in Linux
    #define CLEANUP_SOCKETS() // No need for WSACleanup in Linux
    typedef int socket_t;
#endif

// Function prototypes
int start_client(const char *server_ip, int port);
void communicate_with_server(socket_t sock);

#endif // CLIENT_H
