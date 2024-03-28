#include "log.h"

#include <stdio.h>
#include <ncurses/panel.h>
#include "global.h"

WINDOW* log_win = NULL;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_log_win(int row, int col) {
    int log_width = col / 3;
    log_win = newwin(row - 3, log_width, 0, col - log_width);
    scrollok(log_win, TRUE);
    box(log_win, 0, 0);
}

void log_message(const char *message) {
    pthread_mutex_lock(&log_mutex);

    static int log_line = 1;
    int max_y, max_x;
    getmaxyx(log_win, max_y, max_x);

    if (log_line >= max_y - 1) {
        scroll(log_win);
        log_line--;
    }

    mvwprintw(log_win, log_line, 1, "%.*s", max_x - 2, message); // Print within borders
    printf("%s\n", message);
    log_line++;

    box(log_win, 0, 0);
    update_panels();
    doupdate();
    //wrefresh(log_win);

    log_to_file(message, "INFO");

    pthread_mutex_unlock(&log_mutex);
}

void log_to_file(const char *message, const char *level) {
    //TODO: Looks like file logging breaks things?
    //The sprintf gives a memory access violation?
    /* char* log_name = "log_"; */
    /* sprintf(log_name, "log_%s.txt", uuid_str); */

    FILE *logFile = fopen("LogFile.txt", "a");

    fprintf(logFile, "[%s] : %s\n", level, message);

    fclose(logFile);
}

void destroy_log_win(void) {
    delwin(log_win);
    log_win = NULL;
}
