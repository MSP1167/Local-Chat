#ifndef LIST_H_
#define LIST_H_

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
bool addMessageListItem(MessageList* list, Message message);
void removeOldestItemFromMessageList(MessageList* list);
void freeMessageList(MessageList* list);
Message* getMessageAtIndex(MessageList* list, int index);
bool searchMessageListContainsUUID(MessageList* list, const char* uuid);
void printMessageList(MessageList *list);

#endif // LIST_H_
