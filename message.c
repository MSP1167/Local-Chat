#include "message.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "log.h"
#include "uuid.h"

char* serialize_message(const Message* message) {
    char* buffer = malloc(2048);
    if (buffer == NULL) {
        return NULL; // memory allocation failed
    }

    // create a simple json string
    snprintf(buffer, 4096, "{\"id\": %d, \"username\": \"%s\", \"message\": \"%s\", \"uuid\": \"%s\"}", message->id, message->username, message->message, message->uuid);

    return buffer; // caller is responsible for freeing this memory
}

int deserialize_message(const char* json, Message* message) {
    // Assuming MAX_MESSAGE_LENGTH and MAX_UUID_LENGTH are defined appropriately
    char usernameBuffer[MAX_USERNAME_LENGTH];
    char messageBuffer[MAX_MESSAGE_LENGTH];
    char uuidBuffer[MAX_UUID_LENGTH];
    char log_buf[512]; // Buffer for log messages

    snprintf(log_buf, sizeof(log_buf), "Deserializing Message:\n%s", json);
    log_message(log_buf);
    int parsed = sscanf(json, "{\"id\": %d, \"username\": \"%[^\"]\", \"message\": \"%[^\"]\", \"uuid\": \"%[^\"]\"}",
                        &message->id, usernameBuffer, messageBuffer, uuidBuffer);

    if (parsed == 4) {
        message->server_message = FALSE;
        strncpy(message->username, usernameBuffer, MAX_USERNAME_LENGTH - 1);
        message->username[MAX_USERNAME_LENGTH - 1] = '\0';
        /* log_message("Message Desearialized Successfully"); */
        /* snprintf(log_buf, sizeof(log_buf), "ID: %d", message->id); */
        /* log_message(log_buf); */
        strncpy(message->message, messageBuffer, MAX_MESSAGE_LENGTH - 1);
        //log_message("Message strncpy Successful");
        message->message[MAX_MESSAGE_LENGTH - 1] = '\0';
        /* snprintf(log_buf, sizeof(log_buf), "Message is: %s", messageBuffer); */
        /* log_message(log_buf); */
        strncpy(message->uuid, uuidBuffer, MAX_UUID_LENGTH - 1);
        //log_message("UUID strncpy Successful");
        message->uuid[MAX_UUID_LENGTH - 1] = '\0';
        /* snprintf(log_buf, sizeof(log_buf), "UUID is: %s", uuidBuffer); */
        /* log_message(log_buf); */
        //log_message("Message Allocated Successfully");
        return 1; // Parsing successful
    } else {
        log_message("Message Desearialized Unsuccessful!");
        return 0; // Parsing failed
    }
}
void print_message(const Message* message) {
    printf("Message:\n");
    printf("\tid      : %d\n", message->id);
    printf("\tusername: %s\n", message->username);
    printf("\tmessage : %s\n", message->message);
    printf("\tuuid    : %s\n", message->uuid);
}
