#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <stdbool.h>
#include "uuid.h"

#define  MAX_MESSAGE_LENGTH  1024
#define  MAX_USERNAME_LENGTH 1024
#define  MAX_TIME_LENGTH     1024

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

void print_message(const Message* message);

#endif // MESSAGE_H_
