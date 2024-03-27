#include "uuid.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

/// REMEMBER TO FREE THIS
char* generate_uuid_v4() {
    // Depending on the OS/Enviroment, you may need to srand on each thread
    // To keep it simple, we just srand on every call...
    struct timeval now;
    gettimeofday(&now, NULL);
    srand((unsigned int) now.tv_usec);

    const char *template = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
    const char *hex = "0123456789ABCDEF";

    char *uuid = malloc(37 * sizeof(char)); // Allocate memory for the UUID plus null terminator.

    if (uuid == NULL) {
        return NULL;
    }

    int i = 0;
    while (template[i]) {
        if (template[i] == 'x') {
            uuid[i] = hex[rand() % 16];
        } else if (template[i] == 'y') {
            uuid[i] = hex[(rand() % 4) + 8];
        } else {
            uuid[i] = template[i];
        }
        i++;
    }
    uuid[i] = '\0';
    return uuid;
}
