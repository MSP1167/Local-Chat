#include "user.h"
#include "uuid.h"
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

User* create_user(const char* uuid, const char* username, bool online) {
    User* user = malloc(sizeof(User));

    if (user == NULL) {
        return NULL;
    }

    strcpy_s(user->uuid, MAX_UUID_LENGTH, uuid);
    strcpy_s(user->username, MAX_UUID_LENGTH, username);

    user->online = online;

    struct timeval _now;
    gettimeofday(&_now, NULL);
    time_t now = _now.tv_sec;
    user->lastSeen = now;

    return user;
}

bool is_user_stale(User *user) {
    struct timeval _now;
    gettimeofday(&_now, NULL);

    time_t now = _now.tv_sec;

    if (now - user->lastSeen > 30) {
        return true;
    }
    return false;
}

void free_user(User* user) {
    free(user);
}

void print_user(User* user) {
    printf("User:\n");
    printf("\tuuid    : %s\n", user->uuid);
    printf("\tusername: %s\n", user->username);
    printf("\tonline  : %d\n", user->online);
    printf("\tlastSeen: %lld\n", user->lastSeen);
}
