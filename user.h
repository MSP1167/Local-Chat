#ifndef USER_H_
#define USER_H_

#include <stdbool.h>
#include <time.h>

#include "uuid.h"

#define  MAX_USERNAME_LENGTH 1024

typedef struct User {
    char uuid[MAX_UUID_LENGTH];
    char username[MAX_USERNAME_LENGTH];
    bool online;
    time_t lastSeen;
} User;

User* create_user(const char* uuid, const char* username, bool online);
bool is_user_stale(User* user);
void free_user(User* user);
void print_user(User* user);

#endif // USER_H_
