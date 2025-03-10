#include "server.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

#define MAX_BUFF 80
#define MAX_CLIENT 5
#define RESET(buff)               \
    for (int i = 0; i < MAX_BUFF; i++) \
        buff[i] = '\0';

typedef struct ClientNode {
    SOCKET socket;
    struct ClientNode* next;
} ClientNode;

ClientNode* clients = NULL;
CRITICAL_SECTION clients_lock;

void addClient(SOCKET client) {
    ClientNode* newNode = (ClientNode*)malloc(sizeof(ClientNode));
    newNode->socket = client;
    newNode->next = NULL;

    EnterCriticalSection(&clients_lock);
    if (clients == NULL) {
        clients = newNode;
    } else {
        ClientNode* temp = clients;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
    LeaveCriticalSection(&clients_lock);
}

void removeClient(SOCKET client) {
    EnterCriticalSection(&clients_lock);
    ClientNode* temp = clients;
    ClientNode* prev = NULL;

    while (temp != NULL && temp->socket != client) {
        prev = temp;
        temp = temp->next;
    }

    if (temp != NULL) {
        if (prev == NULL) {
            clients = temp->next;
        } else {
            prev->next = temp->next;
        }
        free(temp);
    }
    LeaveCriticalSection(&clients_lock);
}

void broadcastMessage(SOCKET sender, const char* message) {
    EnterCriticalSection(&clients_lock);
    ClientNode* temp = clients;
    while (temp != NULL) {
        if (temp->socket != sender) {
            send(temp->socket, message, strlen(message), 0);
        }
        temp = temp->next;
    }
    LeaveCriticalSection(&clients_lock);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

unsigned __stdcall receiveMessages(void* connfd_ptr) {
    SOCKET connfd = *(SOCKET*)connfd_ptr;
    char buff[MAX_BUFF];

    while (1) {
        RESET(buff);
        if (recv(connfd, buff, sizeof(buff), 0) > 0) {
            // Print the received message on the left side
            printf("%s\n", buff);
            fflush(stdout);
            broadcastMessage(connfd, buff);
        }
    }
    return 0; // Return a proper value
}

// Function to handle communication with a client
void handleClient(SOCKET connfd, const char* username) {
    char buff[MAX_BUFF];
    int n;

    // Create a thread to receive messages from the client
    HANDLE recv_thread;
    recv_thread = (HANDLE)_beginthreadex(NULL, 0, receiveMessages, &connfd, 0, NULL);

    // Indefinite loop for chatting with the client
    while (1) {
        // Send a response to the client
        RESET(buff);
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
        
        // Prepend "Server: " to the message
        char message[MAX_BUFF];
        snprintf(message, sizeof(message), "Server: %s", buff);
        
        send(connfd, message, strlen(message), 0);
        broadcastMessage(connfd, message);

        // Exit condition
        if (strcmp("Server: exit\n", message) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }

    // Wait for the receive thread to finish
    WaitForSingleObject(recv_thread, INFINITE);
    CloseHandle(recv_thread);

    // Close the connection with the client
    removeClient(connfd);
    CLOSE_SOCKET(connfd);
}

// Main server function
int main(int argc, char const *argv[]) {
    SOCKET sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int clilen;

    // Initialize Windows sockets
    if (INIT_SOCKETS() != 0) {
        error("WSAStartup failed");
    }

    if (argc < 2) {
        error("Port number not provided");
    }

    // Setup server address and port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Create server socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("Error in socket creation");
    }

    // Bind the server socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error("Error in binding");
    }

    // Listen for incoming connections
    if (listen(sockfd, MAX_CLIENT) != 0) {
        error("Error in listening");
    } else {
        printf("Server listening on port %s...\n", argv[1]);
    }

    InitializeCriticalSection(&clients_lock);

    // Accept and handle client connections in separate threads
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            error("Error in accept");
        } else {
            // Receive the username from the client
            char username[MAX_BUFF];
            int bytes_received = recv(newsockfd, username, sizeof(username) - 1, 0);
            if (bytes_received > 0) {
                username[bytes_received] = '\0'; // Null-terminate the received string
                printf("%s joined the chat\n", username);
            }

            // Add the new client to the list
            addClient(newsockfd);

            // Create a new thread to handle communication with the client
            DWORD threadId;
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClient, (LPVOID)newsockfd, 0, &threadId);
        }
    }

    // Cleanup sockets for Windows
    CLEANUP_SOCKETS();
    DeleteCriticalSection(&clients_lock);

    return 0;
}

