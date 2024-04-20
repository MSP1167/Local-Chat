#include "userlist.h"
#include <stdlib.h>
#include "user.h"
#include <string.h>
#include <stdio.h>

UserList* createUserList(int maxItems) {
    UserList* list = (UserList*)malloc(sizeof(UserList));
    if (list == NULL) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->itemCount = 0;
    list->maxItems = maxItems;

    return list;
}

bool addUserListItem(UserList* list, User user) {
    if (list->itemCount >= list->maxItems) {
        return false;
    }

    UserListNode* newNode = (UserListNode*)malloc(sizeof(UserListNode));
    if (newNode == NULL) {
        return false;
    }

    newNode->user = user;
    newNode->next = NULL;

    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }

    list->itemCount++;

    return true;
}

bool removeUserListItem(UserList *list, const char *uuid) {
    UserListNode* temp = list->head;

    if (list->itemCount > 1) {
        while (temp->next != NULL) {
            UserListNode* node = temp->next;
            if (strcmp(node->user.uuid, uuid) == 0) {
                temp->next = node->next;
                if (list->tail == node) {
                    list->tail = temp;
                }
                free(node);
                list->itemCount--;
                return true;
            }
            temp = temp->next;
        }
    } else if (list->itemCount == 1) {
        if (strcmp(temp->user.uuid, uuid) == 0) {
            free(temp);
            list->head = NULL;
            list->tail = NULL;
            list->itemCount--;
            return true;
        }
    }

    return false;
}

void freeUserList(UserList* list) {
    UserListNode* temp = list->head;
    while (temp != NULL) {
        UserListNode* next = temp->next;
        free(temp);
        temp = next;
    }
    free(list);
}

bool searchUserListContainsUUID(UserList *list, const char *uuid) {
    UserListNode* temp = list->head;

    while (temp != NULL) {
        if (strcmp(temp->user.uuid, uuid) == 0) {
            return true;
        }
        temp = temp->next;
    }

    return false;
}

void printUserList(UserList *list) {
    UserListNode* temp = list->head;

    printf("Contents of UserList:\n");

    int i = 0;
    while (temp != NULL) {
        printf("Item %d:\n", i);
        print_user(&temp->user);
        i++;
        temp = temp->next;

    }

}
