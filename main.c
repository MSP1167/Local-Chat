#include <pthread.h>
#include <stdlib.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
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
    //printf("UUID is %s\n", uuid_str);

    // Initialize ncurses
    initscr();
    cbreak(); // Line buffering disabled, pass on everything
    noecho();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);


    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    init_log_win(rows, cols);

#ifdef _WIN32
    WSADATA wsa;

    // Initialise Winsock
    //printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        //printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    //printf("Initialised.\n");
#endif
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
#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
