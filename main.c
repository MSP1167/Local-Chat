#include <pthread.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ncurses/ncurses.h>
#include <stdio.h>
#include <sys/time.h>

#include "server.h"
#include "ui.h"
#include "messagelist.h"
#include "global.h"
#include "log.h"
#include "uuid.h"

char* uuid_str;

int main() {
    // Seed the random number generator with the current time
    struct timeval now;
    gettimeofday(&now, NULL);
    // Use Microseconds
    srand((unsigned int) now.tv_usec);

    // Generate UUID
    uuid_str = generate_uuid_v4();
    printf("UUID is %s\n", uuid_str);

    // Initialize ncurses
    initscr();
    cbreak(); // Line buffering disabled, pass on everything
    noecho();


    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    init_log_win(rows, cols - 30);

    WSADATA wsa;

    // Initialise Winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");

    MessageList* _clientMessages = createMessageList(64);
    pthread_t ui_thread, server_thread;
    if (pthread_create(&ui_thread, NULL, startUI, _clientMessages) < 0) {
        perror("Could not create UI thread!");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&server_thread, NULL, startServer, _clientMessages) < 0) {
        perror("Could not create Server thread!");
        exit(EXIT_FAILURE);
    }

    pthread_join(ui_thread, NULL);
    pthread_join(server_thread, NULL);

    freeMessageList(_clientMessages);
    free(uuid_str);
    WSACleanup();

    return 0;
}
