#include "user.h"
#include "uuid.h"
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __linux__
#pragma GCC diagnostic ignored "-Wunused-parameter"
void strcpy_s(char* dest, int size, const char* src) {
#pragma GCC diagnostic pop
    // TODO: Actually implement this, but for now alias to strcpy
    strcpy(dest, src);
}
#endif

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
    #ifdef _WIN32
    printf("\tlastSeen: %lld\n", user->lastSeen);
    #else
    printf("\tlastSeen: %ld\n", user->lastSeen);
    #endif
}
