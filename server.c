#include "server.h"
#include <windows.h>
#include <time.h>

#define MAX_BUFF 80
#define SA struct sockaddr
#define RESET(buff) \
       { int i; for(i = 0; i < MAX_BUFF; i++) buff[i] = 0; \
           buff[i]='\0'; }

void error (const char *msg) {
    perror(msg);
    exit(1);
}

void readIndefinitely(SOCKET connfd) {
    char buff[MAX_BUFF];
    int a;
    while(1) {
        RESET(buff);
        a = recv(connfd, buff, sizeof(buff), 0);  // Use recv instead of read
        if(a > 0) {
            printf("Client: %s\n", buff);
        }

        // Add response to client
        int n = 0;
        RESET(buff);
        printf("Enter the server response: ");
        while((buff[n++] = getchar()) != '\n');
        send(connfd, buff, strlen(buff), 0);  // Use send instead of write

        // Add exit condition
        if(strcmp("exit\n", buff) == 0) {
            printf("Server Exit...\n");
            break;
        }
    }
}

int main(int argc, char const *argv[]) {
    int MAX_CLIENT = 5;
    SOCKET sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    int clilen;

    // Initialize sockets for Windows
    if (INIT_SOCKETS() != 0) {
        error("WSAStartup failed");
    }

    // Check the provided port number
    if (argc < 2)
        error("Port number not provided");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    // Socket creation
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Error in creating socket");
    else
        printf("Socket created successfully ...\n");

    // Binding
    if (bind(sockfd, (SA *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Error in binding");
    else
        printf("Binding to Port : %s \n", argv[1]);

    // Listening
    if (listen(sockfd, MAX_CLIENT) != 0) {
        error("Error in listening ...");
    } else {
        printf("Started listening for max %d clients...\n", MAX_CLIENT);
    }

    // Accept connections
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (SA *)&cli_addr, (socklen_t *)&clilen);
    if (newsockfd < 0)
        error("Error in accept");
    else
        printf("Accepted the client: %d\n", newsockfd);

    // Start chatting
    readIndefinitely(newsockfd);

    CLOSE_SOCKET(newsockfd);

    // Cleanup sockets for Windows
    CLEANUP_SOCKETS();

    return 0;
}

