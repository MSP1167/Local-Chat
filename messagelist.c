#include "messagelist.h"
#include "message.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>

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

    pthread_cond_broadcast(&list->cond);
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

void freeMessageList(MessageList* list) {
    pthread_mutex_lock(&list->mutex);

    while (list->head != NULL) {
        removeOldestItemFromMessageList(list);
    }

    pthread_mutex_unlock(&list->mutex);

    pthread_mutex_destroy(&list->mutex);
    pthread_cond_destroy(&list->cond);
}

Message* getMessageAtIndex(MessageList* list, int index) {
    pthread_mutex_lock(&list->mutex);

    if (index < 0 || index >= list->itemCount) {
        printf("Could not get message!\n");
        pthread_mutex_unlock(&list->mutex);
        return NULL;
    }

    ListNode* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }

    //printf("Retrieved Message:\n");
    //print_message(&current->message);

    pthread_mutex_unlock(&list->mutex);
    return &(current->message);
}

bool searchMessageListContainsUUID(MessageList *list, const char *uuid) {
    pthread_mutex_lock(&list->mutex);

    ListNode* temp = list->head;

    while (temp != NULL) {
        if (strcmp(temp->message.uuid, uuid) == 0) {
            pthread_mutex_unlock(&list->mutex);
            return true;
        }
        temp = temp->next;
    }

    pthread_mutex_unlock(&list->mutex);
    return false;
}

void printMessageList(MessageList *list) {
    pthread_mutex_lock(&list->mutex);

    ListNode* temp = list->head;

    printf("Contents of MessageList:\n");

    int i = 0;
    while (temp != NULL) {
        printf("Item %d:\n", i);
        print_message(&temp->message);
        i++;
        temp = temp->next;

    }

    pthread_mutex_unlock(&list->mutex);

}
