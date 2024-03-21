#include "list.h"
#include "message.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

MessageList* createMessageList(int maxItems) {
    MessageList* list = (MessageList*)malloc(sizeof(MessageList));
    if (list == NULL) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->itemCount = 0;
    list->maxItems = maxItems;
    list->totalItemsAdded = 0;

    pthread_mutex_init(&list->mutex, NULL);
    pthread_cond_init(&list->cond, NULL);

    return list;
}

bool addMessageListItem(MessageList* list, Message message) {
    pthread_mutex_lock(&list->mutex);

    print_message(&message);

    if (list->itemCount >= list->maxItems) {
        removeOldestItemFromMessageList(list);
    }

    ListNode* newNode = (ListNode*)malloc(sizeof(ListNode));
    if (newNode == NULL) {
        pthread_mutex_unlock(&list->mutex);
        return false;
    }

    newNode->message = message;
    newNode->next = NULL;

    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }

    list->itemCount++;
    list->totalItemsAdded++;

    pthread_cond_signal(&list->cond);
    pthread_mutex_unlock(&list->mutex);

    return true;
}

void removeOldestItemFromMessageList(MessageList *list) {
    if (list->head == NULL) {
        return;
    }

    ListNode* temp = list->head;
    list->head = list->head->next;
    free(temp);

    if (list->head == NULL) {
        list->tail = NULL;
    }

    list->itemCount--;
}

void freeList(MessageList* list) {
    pthread_mutex_lock(&list->mutex);

    while (list->head != NULL) {
        removeOldestItem(list);
    }

    pthread_mutex_unlock(&list->mutex);

    pthread_mutex_destroy(&list->mutex);
    pthread_cond_destroy(&list->cond);
}
