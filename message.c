#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/time.h>

#include "log.h"
#include "uuid.h"

char* serialize_message(const Message* message) {
    char* buffer = malloc(2048);
    if (buffer == NULL) {
        return NULL; // memory allocation failed
    }

    // create a simple json string
    snprintf(buffer, 4096, "{\"id\": %d, \"username\": \"%s\", \"message\": \"%s\", \"time\": \"%s\", \"uuid\": \"%s\"}\n", message->id, message->username, message->message, message->time, message->uuid);

    return buffer; // caller is responsible for freeing this memory
}

int deserialize_message(const char* json, Message* message) {
    // Assuming MAX_MESSAGE_LENGTH and MAX_UUID_LENGTH are defined appropriately
    char usernameBuffer[MAX_USERNAME_LENGTH];
    char messageBuffer[MAX_MESSAGE_LENGTH];
    char timeBuffer[MAX_TIME_LENGTH];
    char uuidBuffer[MAX_UUID_LENGTH];
    char log_buf[512]; // Buffer for log messages

    snprintf(log_buf, sizeof(log_buf), "Deserializing Message:\n%s", json);
    log_message(log_buf);
    int parsed = sscanf(json, "{\"id\": %d, \"username\": \"%[^\"]\", \"message\": \"%[^\"]\", \"time\": \"%[^\"]\", \"uuid\": \"%[^\"]\"}\n",
                        &message->id, usernameBuffer, messageBuffer, timeBuffer, uuidBuffer);

    if (parsed == 5) {
        message->server_message = FALSE;
        strncpy(message->username, usernameBuffer, MAX_USERNAME_LENGTH - 1);
        message->username[MAX_USERNAME_LENGTH - 1] = '\0';

        strncpy(message->message, messageBuffer, MAX_MESSAGE_LENGTH - 1);
        message->message[MAX_MESSAGE_LENGTH - 1] = '\0';

        strncpy(message->time, timeBuffer, MAX_TIME_LENGTH - 1);
        message->message[MAX_TIME_LENGTH - 1] = '\0';

        strncpy(message->uuid, uuidBuffer, MAX_UUID_LENGTH - 1);
        message->uuid[MAX_UUID_LENGTH - 1] = '\0';

        return 1; // Parsing successful
    } else {
        log_message("Message Desearialized Unsuccessful!");
        return 0; // Parsing failed
    }
}

char *convert_time_to_string(const struct tm time) {
    char* buffer = malloc(2048);
    if (buffer == NULL) {
        return NULL;
    }

    strftime(buffer, MAX_TIME_LENGTH, TIME_FORMAT_STR, &time);
    return buffer; // MAKE SURE TO FREE THIS
}

struct tm convert_string_to_time(const char* string) {
    struct tm tm;
    int year, month, day, hour, minute, second;

    if (sscanf(string, "%d-%d-%d %d-%d-%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        tm.tm_year  = year - 1900; // Since 1900
        tm.tm_mon   = month - 1; // Starts at 0
        tm.tm_mday  = day;
        tm.tm_hour  = hour;
        tm.tm_min   = minute;
        tm.tm_sec   = second;
        tm.tm_isdst = -1;       // Daylight Savings Automatic
    }
    return tm;
}


void print_message(const Message* message) {
    printf("Message:\n");
    printf("\tid      : %d\n", message->id);
    printf("\tusername: %s\n", message->username);
    printf("\tmessage : %s\n", message->message);
    printf("\ttime    : %s\n", message->time);
    printf("\tuuid    : %s\n", message->uuid);
}
