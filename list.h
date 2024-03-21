#ifndef MESSAGELIST_H_
#define MESSAGELIST_H_

#include <stdbool.h>
#include <pthread.h>
#include "message.h"

typedef struct ListNode {
    Message message;
    struct ListNode* next;
} ListNode;

typedef struct MessageList {
    ListNode* head;
    ListNode* tail;
    int itemCount;
    int maxItems;
    int totalItemsAdded;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} MessageList;

MessageList *createMessageList(int maxItems);
bool addItem(MessageList* list, Message message);
void removeOldestItemFromMessageList(MessageList* list);
void freeList(MessageList* list);

#endif // MESSAGELIST_H_
