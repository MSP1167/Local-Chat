#include <ncurses/ncurses.h>
#include <ncurses/panel.h>
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
extern WINDOW* log_win;

int screenRows, screenCols;

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

void updateChatBox(MessageList* clientMessages, WINDOW* chatWindow) {
    // Display all messages
    pthread_mutex_lock(&clientMessages->mutex);

    ListNode* temp = clientMessages->head;

    int i = 1;
    while (temp != NULL) {
        //TODO: Message stuff here
        Message current_message = temp->message;
        char* displayMessage = current_message.message;
        if (current_message.id == 10) {
            mvwprintw(chatWindow, i, 1, "%s: %s", current_message.username, displayMessage);
            i++;
        }

        temp = temp->next;
    }
    update_panels();
    doupdate();

    pthread_mutex_unlock(&clientMessages->mutex);
}

void updateUsername(char* username) {
    int box_width = screenCols / 2;
    WINDOW *input_win = newwin(3, box_width, (screenCols / 2) - (box_width / 2), (screenRows / 2) - 2);
    wattron(input_win, COLOR_PAIR(2));

    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, "Input Username");

    keypad(input_win, TRUE);
    wtimeout(input_win, 100);

    PANEL *input_panel = new_panel(input_win);

    update_panels();
    doupdate();

    const int inputOffset = 1;
    char str[MAX_USERNAME_LENGTH] = {0};
    int i = 0;
    while (1) {
        int ch = 0;

        ch = wgetch(input_win);

        if (ch == ERR) {
            // Do nothing for now
        } else if (ch != '\n' && i < MAX_MESSAGE_LENGTH - 1) {
            if (ch == KEY_BACKSPACE || ch == '\b' || ch == 127) { // Handle backspace
                if (i > 0) {
                    i--; // Move back one character
                    mvwaddch(input_win, 1, inputOffset+1+i, ' ');
                }
            } else {
                str[i++] = ch; // Store character and advance
                mvwaddch(input_win, 1, inputOffset+i, ch); // Display the character
            }
            update_panels();
            doupdate();
        }

        if (ch == '\n' || i >= MAX_USERNAME_LENGTH - 1) {
            if (i > 0) {
                str[i] = '\0';

                strcpy(username, str);
                break;
            }
        }
    }
    del_panel(input_panel);
    delwin(input_win);
}

void drawInputWindow(WINDOW* input_win) {
    // Display prompt
    mvwprintw(input_win, 1, 1, "Enter a message: ");
    wclrtoeol(input_win); // Clear the rest of the line
    box(input_win, 0, 0);
}

void* startUI(void* _clientMessages) {
    MessageList *clientMessages = (MessageList*)_clientMessages;
    char messages[MAX_MESSAGES][MAX_MESSAGE_LENGTH] = {0}; // Messages storage
    int message_count = 0; // How many messages have been stored
    char log_buf[512]; // Buffer for log messages
    bool show_log_win = FALSE;

    char username[MAX_USERNAME_LENGTH] = {0}; // Username initialized to empty
    bool usernameSet = FALSE;

    getmaxyx(stdscr, screenRows, screenCols); // Get the number of rows and columns
    //curs_set(0);

    // Windows
    WINDOW *msg_win = newwin(screenRows - 3, screenCols, 0, 0);
    WINDOW *input_win = newwin(3, screenCols, screenRows - 3, 0);

    wattron(msg_win, COLOR_PAIR(1));

    drawInputWindow(input_win);

    box(msg_win, 0, 0);
    mvwprintw(msg_win, 0, 1, "Messages");

    keypad(input_win, TRUE);

    wtimeout(input_win, 10);

    // Panels
    PANEL *msg_panel = new_panel(msg_win);
    PANEL *input_panel = new_panel(input_win);
    PANEL *log_panel = new_panel(log_win);

    if (!show_log_win) {
        hide_panel(log_panel);
    }


    update_panels();
    doupdate();

    int lastMessageRead = 0;
    pthread_mutex_lock(&clientMessages->mutex);
    lastMessageRead = clientMessages->totalItemsAdded - clientMessages->itemCount;
    pthread_mutex_unlock(&clientMessages->mutex);
    char str[MAX_MESSAGE_LENGTH] = {0};
    int i = 0;
    while (1) {
        int ch = 0;
        wattron(input_win, COLOR_PAIR(2));
        ch = wgetch(input_win);

        if (!usernameSet) {
            wattroff(input_win, COLOR_PAIR(2));
            wattron(input_win, COLOR_PAIR(1));
            touchwin(input_win);
            update_panels();
            doupdate();

            updateUsername(username);
            usernameSet = TRUE;
            wattroff(input_win, COLOR_PAIR(1));
            wattron(input_win, COLOR_PAIR(2));
            drawInputWindow(input_win);
            update_panels();
            doupdate();
        }

        if (ch == ERR) {
            // This means timeout happened
            pthread_mutex_lock(&clientMessages->mutex);

            int totalItemsAdded = clientMessages->totalItemsAdded;

            while (lastMessageRead < totalItemsAdded) {
                snprintf(log_buf, sizeof(log_buf), "Adding messages... %d -> %d", lastMessageRead, totalItemsAdded);
                log_message(log_buf);
                pthread_mutex_unlock(&clientMessages->mutex);
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

                strncpy(messages[message_count], message, strlen(message));
                log_message("Displaying Messages...");
                message_count = (message_count + 1) % MAX_MESSAGES; // Ensure we don't go out of bounds
                pthread_mutex_unlock(&clientMessages->mutex);
                updateChatBox(clientMessages, msg_win);
                pthread_mutex_lock(&clientMessages->mutex);
                lastMessageRead++;
            }
            pthread_mutex_unlock(&clientMessages->mutex);
        } else if (ch == KEY_F(9)) {
            if (show_log_win) {
                hide_panel(log_panel);
            }
            else {
                show_panel(log_panel);
            }
            show_log_win = show_log_win ? FALSE : TRUE;

            update_panels();
            doupdate();
        } else if (ch == KEY_F(10)) {
            sendBroadcast();
            //TODO: Add a hotkey to print out the contents of the internal clientMessages
            //buffer, probablu to the log
        } else if (ch != '\n' && i < MAX_MESSAGE_LENGTH - 1) {
            if (ch == KEY_BACKSPACE || ch == '\b' || ch == 127) { // Handle backspace
                if (i > 0) {
                    i--; // Move back one character
                    mvwaddch(input_win, 1, 18+i, ' ');
                }
            } else {
                str[i++] = ch; // Store character and advance
                mvwaddch(input_win, 1, 17+i, ch); // Display the character
            }
            update_panels();
            doupdate();
        }

        if (ch == '\n' || i >= MAX_MESSAGE_LENGTH - 1) {
            if (i > 0) {
                str[i] = '\0';

                if(strcmp(str, "exit") == 0) break;

                // Add message to global list
                Message message = {10, FALSE, "", "", ""};
                message.id = 10;
                strncpy(message.username, username, MAX_USERNAME_LENGTH);
                strncpy(message.message, str, MAX_MESSAGE_LENGTH);
                strncpy(message.uuid, generate_uuid_v4(), MAX_UUID_LENGTH);
                print_message(&message);
                addMessageListItem(clientMessages, message);
                strncpy(messages[message_count], str, MAX_MESSAGE_LENGTH);
                message_count = (message_count + 1) % MAX_MESSAGES; // Ensure we don't go out of bounds

                updateChatBox(clientMessages, msg_win);

                i = 0;
                memset(str, 0, sizeof str); // Clear the input string

                // Display prompt
                drawInputWindow(input_win);
                update_panels();
                doupdate();
            }
        }
    }
    printf("Exiting...\n");

    // Cleanup
    del_panel(msg_panel);
    del_panel(input_panel);
    del_panel(log_panel);
    delwin(msg_win);
    delwin(input_win);
    delwin(log_win);
    endwin();

    exit(0);
}
