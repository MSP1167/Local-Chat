#ifndef LOG_H_
#define LOG_H_

#include <pthread.h>
#include <ncurses/ncurses.h>

extern WINDOW *log_win;
extern pthread_mutex_t log_mutex;

void init_log_win(int row, int col);
void log_message(const char *message);
void log_to_file(const char *message, const char *level);
void destroy_log_win(void);

#endif // LOG_H_
