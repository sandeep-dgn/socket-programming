#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>  // Required for Winsock functions

#define MAX_BUFF 80
#define SA struct sockaddr
#define RESET(buff)               \
    for (int i = 0; i < MAX_BUFF; i++) \
        buff[i] = '\0';

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void readIndefinitely(SOCKET connfd)
{
    char buffer[MAX_BUFF];
    int n;

    // Indefinite loop for chat
    while (1)
    {
        // Clear the buffer
        RESET(buffer);

        // Send the data
        n = 0;
        printf("Enter the data: ");
        while ((buffer[n++] = getchar()) != '\n');
        send(connfd, buffer, strlen(buffer), 0);  // Use send instead of write

        // Add exit condition
        if (strcmp("exit\n", buffer) == 0)
        {
            printf("Client Exit ...\n");
            break;
        }

        // Read the data from server into buffer
        if (recv(connfd, buffer, sizeof(buffer), 0) > 0)  // Use recv instead of read
        {
            printf("From Server : %s", buffer);
        }
    }
}

int main(int argc, char const *argv[])
{
    SOCKET sockfd;
    struct sockaddr_in serv_addr;

    // Initialize sockets for Windows
    if (INIT_SOCKETS() != 0) {
        error("WSAStartup failed");
    }

    // Checking the provided port no
    if (argc < 3)
        error("Port No not provided");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Socket creation
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Error in Socket creation");

    // Connect to server
    if (connect(sockfd, (SA *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error in Connecting to server ..");

    // Start communication
    readIndefinitely(sockfd);

    // Close the socket
    CLOSE_SOCKET(sockfd);

    // Cleanup sockets for Windows
    CLEANUP_SOCKETS();

    return 0;
}
