#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __linux__
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#include "server.h"
#include "message.h"
#include "messagelist.h"
#include "global.h"
#include "log.h"

#define BUFFER_SIZE 2048
#define MAX_MESSAGES 64

// #define SELF_CONNECT

// arg struct for main_thread_write
typedef struct main_thread_struct {
    SOCKET* socket_desc;
    struct sockaddr_in client;
    bool should_exit;
    pthread_mutex_t mutex;
} main_thread_struct;

MessageList* clientMessages;

int server_port;

void* handle_main_connection_write(void* main_thread_args) {
    main_thread_struct *args = (main_thread_struct*)main_thread_args;
    int sock = *(args->socket_desc);
    // struct sockaddr_in client = args->client;
    int last_message_read = 0;

    pthread_mutex_lock(&clientMessages->mutex);
    last_message_read = clientMessages->totalItemsAdded - clientMessages->itemCount;
    pthread_mutex_unlock(&clientMessages->mutex);

    log_message("Waiting to send data to new peer...");
    while(1) {
        pthread_mutex_lock(&clientMessages->mutex);
        while(last_message_read >= clientMessages->totalItemsAdded) {
            // This unlocks the mutex and waits for the condition to be alive
            // It will then relock the mutex
            pthread_cond_wait(&clientMessages->cond, &clientMessages->mutex);
        }
        // Save the total message count
        int totalItemsAdded = clientMessages->totalItemsAdded;
        pthread_mutex_unlock(&clientMessages->mutex);

        // Check if we should exit
        pthread_mutex_lock(&args->mutex);
        if (args->should_exit) {
            pthread_mutex_unlock(&args->mutex);
            log_message("Writer Thread Got Exit Signal, Exiting");
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&args->mutex);

        // Lock the args mutex to use the socket
        while(last_message_read < totalItemsAdded) {
            // send the data
            Message* message_to = getMessageAtIndex(clientMessages, last_message_read);
            if (message_to == NULL) {
                log_message("Message was null; Skipping...");
                last_message_read++;
                continue;
            }
            pthread_mutex_lock(&clientMessages->mutex);
            char* message = serialize_message(message_to);
            pthread_mutex_unlock(&clientMessages->mutex);

            pthread_mutex_lock(&args->mutex);
            //printf("Sending Message %s\n", message);
            if (send(sock, message, strlen(message), 0) < 0) {
                log_message("Sent Message");
                free(message);
                closesocket(sock);
                pthread_mutex_unlock(&args->mutex);
                return NULL;
            }
            pthread_mutex_unlock(&args->mutex);
            free(message);

            last_message_read++;
        }
    }

    return NULL;
}


// Handler for each client connection
void* handle_main_connection_read(void* main_thread_args) {
    main_thread_struct *args = (main_thread_struct*)main_thread_args;
    SOCKET sock = *(args->socket_desc);
    int read_size;
    char client_message[BUFFER_SIZE] = {0};

    // Receive a message from client
    // Message could be concatinated b/c of multiple messages in a row stuck in the recv buffer
    while((read_size = recv(sock, client_message, BUFFER_SIZE, 0)) > 0) {
        log_message("Got Data!");
        log_message(client_message);

        char* start = client_message;
        char* end   = strchr(start, '\n');
        while (end != NULL) {
            *end = '\0';
            Message received;
            deserialize_message(start, &received);

            pthread_mutex_lock(&args->mutex);
            if (args->should_exit) {
                pthread_mutex_unlock(&args->mutex);
                closesocket(sock);
                // free(args->socket_desc);
                //printf("Exiting Main Connection Read Thread...\n");
                return NULL;
            }
            pthread_mutex_unlock(&args->mutex);

            // Handle the request
            if (received.id == 10 || received.id == 11 || received.id == 21 || received.id == 31) {
                if (!searchMessageListContainsUUID(clientMessages, received.uuid)) {
                    log_message("Do not have this message, adding to list");
                    // Handles mutex and cond signalling
                    addMessageListItem(clientMessages, received);
                }
                else {
                    log_message("Already have this message, skipping...");
                }
            }

            start = end + 1;
            end = strchr(start, '\n');
        }
        if (*start != '\0') {
            memmove(client_message, start, strlen(start) + 1);
        } else {
            memset(client_message, 0, BUFFER_SIZE);
        }
    }

    if(read_size == 0) {
        //puts("Client disconnected");
    } else if(read_size == SOCKET_ERROR) {
        #ifdef _WIN32
        //printf("Main connection recv failed : %d\n", WSAGetLastError());
        #elif __linux__
        perror("Main connection recv failed");
        #endif
    }

    //printf("Closing socket\n");
    closesocket(sock);
    pthread_mutex_lock(&args->mutex);
    //printf("Should_exit is going on\n");
    args->should_exit = true;
    pthread_mutex_unlock(&args->mutex);
    //TODO: Program seems to crash after this? It exits, so idk
    pthread_exit(NULL);
    //printf("Thread should have exit!");
    return NULL;
}

void *listen_on_port_tcp(void *server_socket_pointer) {
    int server_socket = *(int*)server_socket_pointer;
    int client_sock;
    socklen_t c = sizeof(struct sockaddr_in);
    struct sockaddr_in client;

    while((client_sock = accept(server_socket, (struct sockaddr *)&client, &c)) != (int)INVALID_SOCKET) {
        //puts("Connection accepted");

        SOCKET* new_sock = malloc(sizeof(SOCKET));
        *new_sock = client_sock;

        main_thread_struct* main_thread_args;
        main_thread_args = malloc(sizeof(main_thread_struct));

        main_thread_args->socket_desc = new_sock;
        main_thread_args->client = client;
        pthread_mutex_init(&main_thread_args->mutex, NULL);
        main_thread_args->should_exit = false;

        pthread_t thread_read, thread_write;
        if(pthread_create(&thread_read, NULL, handle_main_connection_read, (void*)main_thread_args) < 0) {
            perror("could not create main read thread");
            free(new_sock);
            free(main_thread_args);
            return (void*)1;
        }
        if(pthread_create(&thread_write, NULL, handle_main_connection_write, (void*)main_thread_args) < 0) {
            perror("could not create main read thread");
            free(new_sock);
            free(main_thread_args);
            return (void*)1;
        }
        //printf("Started both threads!\n");
    }
    //printf("listen_on_port_tcp exited, this should not be!\n");
    perror("Error?");
    return 0;
}

void* handle_broadcast_connection(void* socket_desc) {
    int sock = *(int*)socket_desc;
    struct sockaddr_in si_other;
    socklen_t slen = sizeof(si_other);
    char buffer[BUFFER_SIZE] = {0};
    int read_size;
    char log_buf[512];
    log_message("Broadcast Server Started...");

    snprintf(log_buf, sizeof(log_buf), "Using Socket: %d", sock);
    log_message(log_buf);

    while(1) {
        log_message("Broadcast Waiting on Message");
        // Receive broadcast message
        if ((read_size = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&si_other, &slen)) < 0) {
            // Exit if we read error
            perror("Broadcast Recv Error");
            closesocket(sock);
            return NULL;
        }
        log_message("Got Broadcast\n");

        buffer[read_size] = '\0';

        char* senderUUID = strtok(buffer, ":");
        int senderPort = atoi(strtok(NULL, ":"));
        snprintf(log_buf, sizeof(log_buf), "Got UUID: %s | PORT: %d\n", senderUUID, senderPort);
        log_message(log_buf);

        if (strcmp(senderUUID, uuid_str) == 0) {
            log_message("Got Broadcast from Self\n");
            #if !defined(SELF_CONNECT)
            continue;
            #endif
        }
        log_message("Got Broadcast from Other\n");

        //printf("Creating new socket...");
        SOCKET new_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (new_sock == INVALID_SOCKET) {
            log_message("Could not create com socket!");
            continue;
        }
        //printf("Done!\n");

        struct sockaddr_in peer_addr;
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_addr.s_addr = si_other.sin_addr.s_addr;
        peer_addr.sin_port = htons(senderPort);

        //printf("Peer IP:   %s\n", inet_ntoa(peer_addr.sin_addr));
        //printf("Peer PORT: %d\n", senderPort);

        //printf("Connecting to server...");
        // Connect to server
        if (connect(new_sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
            perror("Failed connecting to server");
            continue;
        }
        //printf("Done!\n");
        log_message("Connected to new peer!");

        main_thread_struct* main_thread_args;
        main_thread_args = malloc(sizeof(main_thread_struct));

        main_thread_args->socket_desc = &new_sock;
        main_thread_args->client = peer_addr;
        pthread_mutex_init(&main_thread_args->mutex, NULL);
        main_thread_args->should_exit = false;

        pthread_t sniffer_thread_read, sniffer_thread_write;
        if(pthread_create(&sniffer_thread_read, NULL, handle_main_connection_read, (void*)main_thread_args) < 0) {
            perror("could not create main read thread");
            /* free(new_sock); */
            free(main_thread_args);
            return (void*)1;
        }
        if(pthread_create(&sniffer_thread_write, NULL, handle_main_connection_write, (void*)main_thread_args) < 0) {
            perror("could not create main write thread");
            /* free(new_sock); */
            free(main_thread_args);
            return (void*)1;
        }
        //printf("Started both threads!\n");
    }
    //printf("While loop broke, ???\n");
    return NULL;
}

void* startServer(void* _clientMessages) {
    // Winsock should already be initialized
    // WSADATA wsa;
    SOCKET server_socket, broadcast_socket; //client_sock, ;
    struct sockaddr_in server, broadcast;
    //int c;

    clientMessages = (MessageList*)_clientMessages;

    // Create sockets
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        #ifdef _WIN32
        printf("Could not create socket : %d", WSAGetLastError());
        #else
        perror("Could not create socket");
        #endif
    }
    if((broadcast_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        #ifdef _WIN32
        printf("Could not create socket : %d", WSAGetLastError());
        #else
        perror("Could not create socket");
        #endif
    }
    //printf("Sockets created.\n");

    // Setup Sock Options on Broadcast Socket
    int broadcastPermission = 1;
    if (setsockopt(broadcast_socket, SOL_SOCKET, SO_BROADCAST, (void*)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("Failed to set broadcast permission");
        closesocket(broadcast_socket);
        exit(ERR);
    }

    int reuseSocket = 1;
    if (setsockopt(broadcast_socket, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseSocket, sizeof(reuseSocket)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        closesocket(broadcast_socket);
        exit(ERR);
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htons(INADDR_ANY);
    server.sin_port = htons(0);

    broadcast.sin_family = AF_INET;
    broadcast.sin_addr.s_addr = htons(INADDR_ANY);
    broadcast.sin_port = htons(8081);

    // Bind
    if(bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        #ifdef _WIN32
        //printf("Bind failed with error code : %d", WSAGetLastError());
        #else
        perror("Bind failed with error code");
        #endif
        exit(EXIT_FAILURE);
    }
    // Set Server Port Variable
    socklen_t server_len = sizeof(server);
    if (getsockname(server_socket, (struct sockaddr *)&server, &server_len) == -1) {
        perror("Failed to get port of server");
        exit(EXIT_FAILURE);
    }
    server_port = ntohs(server.sin_port);
    if(bind(broadcast_socket, (struct sockaddr *)&broadcast, sizeof(broadcast)) == SOCKET_ERROR) {
        #ifdef _WIN32
        //printf("Bind failed with error code : %d", WSAGetLastError());
        #else
        perror("Bind failed with error code");
        #endif
        exit(EXIT_FAILURE);
    }
    //puts("Bind done");


    // Listen to incoming connections
    listen(server_socket, 3);

    // Accept and incoming connection
    //puts("Waiting for incoming connections...");
    // c = sizeof(struct sockaddr_in);

    pthread_t main_thread, broadcast_thread;

    if (pthread_create(&main_thread, NULL, listen_on_port_tcp, (void*)&server_socket) < 0){
        perror("Could not create server thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&broadcast_thread, NULL, handle_broadcast_connection, (void*)&broadcast_socket) < 0){
        perror("Could not create broadcast thread!");
        exit(EXIT_FAILURE);
    }

    pthread_join(main_thread, NULL);
    pthread_join(broadcast_thread, NULL);


    closesocket(server_socket);
    closesocket(broadcast_socket);

    return 0;
}

/* int main() { */
/*     WSADATA wsa; */

/*     // Initialize Client Data Struct */
/*     clientMessages = createMessageList(MAX_MESSAGES); */

/*     // Initialise Winsock */
/*     printf("\nInitialising Winsock..."); */
/*     if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) { */
/*         printf("Failed. Error Code : %d", WSAGetLastError()); */
/*         return 1; */
/*     } */
/*     printf("Initialised.\n"); */

/*     MessageList* _clientMessages = createMessageList(MAX_MESSAGES); */
/*     startServer(_clientMessages); */

/*     return 0; */
/* } */
