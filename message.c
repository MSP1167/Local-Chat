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
    snprintf(buffer, 2048, "{\"id\": %d, \"message\": \"%s\", \"uuid\": \"%s\"}", message->id, message->message, message->uuid);

    return buffer; // caller is responsible for freeing this memory
}

int deserialize_message(const char* json, Message* message) {
    // Assuming MAX_MESSAGE_LENGTH and MAX_UUID_LENGTH are defined appropriately
    char messageBuffer[MAX_MESSAGE_LENGTH];
    char uuidBuffer[MAX_UUID_LENGTH];
    char log_buf[512]; // Buffer for log messages

    snprintf(log_buf, sizeof(log_buf), "Deserializing Message:\n%s", json);
    log_message(log_buf);
    int parsed = sscanf(json, "{\"id\": %d, \"message\": \"%[^\"]\", \"uuid\": \"%[^\"]\"}",
                        &message->id, messageBuffer, uuidBuffer);

    if (parsed == 3) {
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
    printf("\tid     : %d\n", message->id);
    printf("\tmessage: %s\n", message->message);
    printf("\tuuid   : %s\n", message->uuid);
}
