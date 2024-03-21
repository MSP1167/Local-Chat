#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <pthread.h>

#include "message.h"
#include "clientqueue.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_SERVERS 64

// Server Management
typedef struct ServerList {
    char server_ips[MAX_SERVERS][INET_ADDRSTRLEN];
    int server_count;
    pthread_mutex_t mutex;
} ServerList;

ServerList serverList = {
.server_count = 0,
.mutex = PTHREAD_MUTEX_INITIALIZER
};

// Broadcast stuff
void broadcast_port(int port) {
    SOCKET sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    char message[12];
    sprintf(message, "%d", port);
    if(sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Sendto failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    closesocket(sock);
}

void* handle_broadcast_response(void* socket_desc) {
    int sock = *(int*)socket_desc;
    struct sockaddr_in si_other;
    int slen = sizeof(si_other);
    char buffer[BUFFER_SIZE] = {0};
    int read_size;

    while(1) {
        if ((read_size = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&si_other, &slen)) == -1) {
            perror("recvfrom failed");
            closesocket(sock);
            return NULL;
        }

        char ipAddress[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(si_other.sin_addr), ipAddress, INET_ADDRSTRLEN);

        pthread_mutex_lock(&serverList.mutex);

        if (serverList.server_count < MAX_SERVERS) {
            strncpy(serverList.server_ips[serverList.server_count], ipAddress, BUFFER_SIZE);
            serverList.server_count++;

            puts("Got new Peer: ");
            puts(ipAddress);
        }

        pthread_mutex_unlock(&serverList.mutex);
    }

    return NULL;
}

void* initialize_client(ClientQueue* queue) {
    // WSA better be initialized
    SOCKET sock;
    struct sockaddr_in server;
    int server_reply_length = 2000;
    char server_reply[2000] = {0};
    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d" , WSAGetLastError());
        WSACleanup();
        return NULL;
    }

    printf("Socket created.\n");

    // Prepare the sockaddr_in structure
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP
    server.sin_family = AF_INET;
    server.sin_port = htons(8080); // Server port

    //Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("connect error");
        closesocket(sock);
        return NULL;
    }

    printf("Connected\n");

    char* exit_command = "exit";
    while(1) {
        char* data = ClientDequeue(queue);
        if (strcmp(data, exit_command) == 0)
            break;
        Message message_to = {10, ""};
        strncpy(message_to.message, data, 1024);
        char* message = serialize_message(&message_to);

        // Send some data
        if (send(sock, message, strlen(message), 0) < 0) {
            printf("Send failed");
            free(message);
            closesocket(sock);
            return NULL;
        }

        free(message);

        //Receive a reply from the server
        if (recv(sock, server_reply, server_reply_length, 0) < 0) {
            puts("recv failed");
        }

        Message message_from;
        deserialize_message(server_reply, &message_from);

        /* puts(server_reply); */
        //print_example(&message_from);
    }

    closesocket(sock);
    return (int*)1;
}

/* int main(int argc, char *argv[]) { */
/*     WSADATA wsaData; */
/*     SOCKET sock; */
/*     struct sockaddr_in server; */
/*     int server_reply_length = 2000; */
/*     char server_reply[2000] = {0}; */

/*     // Initialize Winsock */
/*     printf("Initializing Winsock...\n"); */
/*     if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) { */
/*         printf("Failed. Error Code: %d", WSAGetLastError()); */
/*         return 1; */
/*     } */

/*     printf("Initialized.\n"); */

/*     // Create a socket */
/*     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) { */
/*         printf("Could not create socket: %d" , WSAGetLastError()); */
/*         WSACleanup(); */
/*         return 1; */
/*     } */

/*     printf("Socket created.\n"); */

/*     // Prepare the sockaddr_in structure */
/*     server.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP */
/*     server.sin_family = AF_INET; */
/*     server.sin_port = htons(8080); // Server port */

/*     //Connect to remote server */
/*     if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) { */
/*         printf("connect error"); */
/*         closesocket(sock); */
/*         WSACleanup(); */
/*         return 1; */
/*     } */

/*     printf("Connected\n"); */

/*     char input[1024] = {0}; */
/*     char* exit_command = "exit"; */
/*     while(1) { */
/*         printf("Message to send: "); */
/*         scanf("%s", input); */
/*         if (strcmp(input, exit_command) == 0) */
/*             break; */
/*         Message message_to = {10, ""}; */
/*         strncpy(message_to.message, input, 1024); */
/*         char* message = serialize_message(&message_to); */

/*         // Send some data */
/*         if (send(sock, message, strlen(message), 0) < 0) { */
/*             printf("Send failed"); */
/*             free(message); */
/*             closesocket(sock); */
/*             WSACleanup(); */
/*             return 1; */
/*         } */
/*         //printf("Sent:\n"); */
/*         //print_example(&message_to); */

/*         free(message); */

/*         //Receive a reply from the server */
/*         if (recv(sock, server_reply, server_reply_length, 0) < 0) { */
/*             puts("recv failed"); */
/*         } */

/*         Message message_from; */
/*         deserialize_message(server_reply, &message_from); */

/*         puts("Server reply:"); */
/*         /\* puts(server_reply); *\/ */
/*         //print_example(&message_from); */
/*     } */

/*     closesocket(sock); */
/*     WSACleanup(); */

/*     return 0; */
/* } */
