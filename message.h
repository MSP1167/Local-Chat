#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdbool.h>
#include <sys/time.h>
#include "uuid.h"
#include "user.h"

#define  MAX_MESSAGE_LENGTH  1024
#define  MAX_TIME_LENGTH     1024

#define TIME_FORMAT_STR      "%Y-%m-%d %H-%M-%S"
#define TIME_UNFROMAT_STR    "%d-%d-%d %d-%d-%d"
#define TIME_PRETTY_STR      "%d-%d-%d %d:%d:%d"

typedef struct Message {
    int id;
    bool server_message;
    char username[MAX_USERNAME_LENGTH];
    char message[MAX_MESSAGE_LENGTH];
    char time[MAX_MESSAGE_LENGTH];
    char uuid[MAX_UUID_LENGTH];
} Message;

char *serialize_message(const Message* message);

int deserialize_message(const char* json, Message* message);

char *convert_time_to_string(const struct tm time);

struct tm convert_string_to_time(const char* string);

void print_message(const Message* message);

#endif // MESSAGE_H_
