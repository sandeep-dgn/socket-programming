#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

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

DWORD WINAPI receiveMessages(LPVOID connfd_ptr) {
    SOCKET connfd = *(SOCKET*)connfd_ptr;
    char buffer[MAX_BUFF];

    while (1) {
        RESET(buffer);
        if (recv(connfd, buffer, sizeof(buffer), 0) > 0) {
            // Print the received message on the left side
            printf("%s\n", buffer);
            fflush(stdout);
        }
    }
    return 0; // Return a proper value
}

void readIndefinitely(SOCKET connfd, const char* username)
{
    char buffer[MAX_BUFF];
    int n;

    // Send the username to the server
    send(connfd, username, strlen(username), 0);

    // Create a thread to receive messages from the server
    HANDLE recv_thread;
    recv_thread = CreateThread(NULL, 0, receiveMessages, &connfd, 0, NULL);

    // Indefinite loop for chat
    while (1)
    {
        // Clear the buffer
        RESET(buffer);

        // Send the data
        n = 0;
        while ((buffer[n++] = getchar()) != '\n');
        
        // Prepend the username to the message
        char message[MAX_BUFF];
        snprintf(message, sizeof(message), "%s: %s", username, buffer);
        
        send(connfd, message, strlen(message), 0);

        // Add exit condition
        if (strcmp("exit\n", buffer) == 0)
        {
            printf("Client Exit ...\n");
            break;
        }
    }

    // Wait for the receive thread to finish
    WaitForSingleObject(recv_thread, INFINITE);
    CloseHandle(recv_thread);
}

int main(int argc, char const *argv[])
{
    SOCKET sockfd;
    struct sockaddr_in serv_addr;
    char username[MAX_BUFF];

    // Initialize sockets for Windows
    if (INIT_SOCKETS() != 0) {
        error("WSAStartup failed");
    }

    // Checking the provided port no
    if (argc < 3)
        error("Port No not provided");

    // Prompt for username
    printf("Enter your username: ");
    fgets(username, MAX_BUFF, stdin);
    username[strcspn(username, "\n")] = 0; // Remove newline character

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
    readIndefinitely(sockfd, username);

    // Close the socket
    CLOSE_SOCKET(sockfd);

    // Cleanup sockets for Windows
    CLEANUP_SOCKETS();

    return 0;
}

