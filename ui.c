#include <ncurses/ncurses.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>

#include "ui.h"
#include "message.h"
#include "messagelist.h"
#include "global.h"
#include "log.h"

#define MAX_MESSAGES 100

#define BROADCAST_IP "127.255.255.255"
#define BROADCAST_PORT 8081

extern int server_port;

void sendBroadcast() {
    log_message("Sending Broadcast...");
    SOCKET sock;
    struct sockaddr_in broadcastAddr;
    int broadcastPermission = 1;
    char log_buf[512];

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) {
        perror("Failed to create socket for broadcast");
        return;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void*)&broadcastPermission, sizeof(broadcastPermission)) < 0) {
        perror("Failed to set broadcast permission");
        closesocket(sock);
        return;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    broadcastAddr.sin_port = htons(BROADCAST_PORT);

    char message[64];
    sprintf_s(message, sizeof(message), "%s:%d", uuid_str, server_port);

    if (sendto(sock, message, sizeof(message), 0, (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0) {
        perror("Failed to send port");
    }
    closesocket(sock);
    snprintf(log_buf, sizeof(log_buf), "Sent Broadcast %s", message);
    log_message(log_buf);
    snprintf(log_buf, sizeof(log_buf), "\tto %s:%d", BROADCAST_IP, BROADCAST_PORT);
    log_message(log_buf);
}


void* startUI(void* _clientMessages) {
    MessageList *clientMessages = (MessageList*)_clientMessages;
    char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH] = {0}; // Messages storage
    int message_count = 0; // How many messages have been stored
    char log_buf[512]; // Buffer for log messages

    int row, col;
    getmaxyx(stdscr, row, col); // Get the number of rows and columns

    // Windows
    WINDOW *msg_win = newwin(row - 3, col, 0, 0);
    WINDOW *input_win = newwin(3, col, row - 3, 0);

    box(msg_win, 0, 0);
    box(input_win, 0, 0);
    wrefresh(msg_win);
    wrefresh(input_win);
    wrefresh(log_win);

    keypad(input_win, TRUE);

    wtimeout(input_win, 100);

    // Display prompt
    mvwprintw(input_win, 1, 1, "Enter a message: ");
    wclrtoeol(input_win); // Clear the rest of the line
    box(input_win, 0, 0);
    wrefresh(input_win);

    int lastMessageRead = 0;
    pthread_mutex_lock(&clientMessages->mutex);
    lastMessageRead = clientMessages->totalItemsAdded - clientMessages->itemCount;
    pthread_mutex_unlock(&clientMessages->mutex);
    char str[MAX_MESSAGE_LENGTH] = {0};
    int i = 0;
    while (1) {
        int ch = 0;

        // log_message("Getting character...");
        ch = wgetch(input_win);
        // snprintf(log_buf, sizeof(log_buf), "Got Character %c (%d)", ch, ch);
        // log_message(log_buf);

        if (ch == ERR) {
            // This means timeout happened
            // log_message("Doing Downtime");
            pthread_mutex_lock(&clientMessages->mutex);

            int totalItemsAdded = clientMessages->totalItemsAdded;

            while (lastMessageRead < totalItemsAdded) {
                snprintf(log_buf, sizeof(log_buf), "Adding messages... %d -> %d", lastMessageRead, totalItemsAdded);
                log_message(log_buf);
                pthread_mutex_unlock(&clientMessages->mutex);
                //log_message("Getting Messages...");
                // TODO: Problem with this, if we start over the max then this will fail
                Message* temp = getMessageAtIndex(clientMessages, lastMessageRead);
                char* message = temp->message;

                pthread_mutex_lock(&clientMessages->mutex);
                if (message == NULL) {
                    log_message("Message was null, skipping...");
                    lastMessageRead++;
                    continue;
                }
                print_message(temp);

                /* log_message("Got Message"); */
                /* snprintf(log_buf, sizeof(log_buf), "strncpy with params:"); */
                /* log_message(log_buf); */
                /* snprintf(log_buf, sizeof(log_buf), "\tmessages[%d]", message_count); */
                /* log_message(log_buf); */
                /* snprintf(log_buf, sizeof(log_buf), "\t%s", message); */
                /* log_message(log_buf); */
                /* snprintf(log_buf, sizeof(log_buf), "\t%d", strlen(message)); */
                /* log_message(log_buf); */
                strncpy(messages[message_count], message, strlen(message));
                log_message("Displaying Messages...");
                message_count = (message_count + 1) % MAX_MESSAGES; // Ensure we don't go out of bounds
                // Display all messages
                for (int j = 0; j < message_count; j++) {
                    mvwprintw(msg_win, j + 1, 1, "%s", messages[j]);
                }
                lastMessageRead++;
            }
            pthread_mutex_unlock(&clientMessages->mutex);
            wrefresh(msg_win);

        } else if (ch == KEY_F(10)) {
            sendBroadcast();
            //TODO: Add a hotkey to print out the contents of the internal clientMessages
            //buffer, probablu to the log
        } else if (ch != '\n' && i < MAX_MESSAGE_LENGTH - 1) {
            if (ch == KEY_BACKSPACE || ch == '\b' || ch == 127) { // Handle backspace
                if (i > 0) {
                    i--; // Move back one character
                    mvwdelch(input_win, 1, 18+i); // Delete the displayed character
                }
            } else {
                str[i++] = ch; // Store character and advance
                mvwaddch(input_win, 1, 17+i, ch); // Display the character
            }
            wrefresh(input_win);
        }

        if (ch == '\n' || i >= MAX_MESSAGE_LENGTH - 1) {
            if (i > 0) {
                str[i] = '\0';

                if(strcmp(str, "exit") == 0) break;

                // Add message to global list
                Message message = {10, "", ""};
                message.id = 10;
                strncpy(message.message, str, MAX_MESSAGE_LENGTH);
                strncpy(message.uuid, generate_uuid_v4(), MAX_UUID_LENGTH);
                print_message(&message);
                addMessageListItem(clientMessages, message);
                // printMessageList(clientMessages);
                strncpy(messages[message_count], str, MAX_MESSAGE_LENGTH);
                message_count = (message_count + 1) % MAX_MESSAGES; // Ensure we don't go out of bounds

                // Display all messages
                for (int j = 0; j < message_count; j++) {
                    mvwprintw(msg_win, j + 1, 1, "%s", messages[j]);
                }
                wrefresh(msg_win);

                i = 0;
                memset(str, 0, sizeof str); // Clear the input string

                // Display prompt
                mvwprintw(input_win, 1, 1, "Enter a message: ");
                wclrtoeol(input_win); // Clear the rest of the line
                box(input_win, 0, 0);
                wrefresh(input_win);
            }
        }
    }
    printf("Exiting...\n");

    // Cleanup
    delwin(msg_win);
    delwin(input_win);
    endwin();

    exit(0);
}
