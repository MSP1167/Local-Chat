#ifndef USERLIST_H_
#define USERLIST_H_

#include <stdbool.h>
#include "user.h"

typedef struct UserListNode {
    User user;
    struct UserListNode* next;
} UserListNode;

typedef struct UserList {
    UserListNode* head;
    UserListNode* tail;
    int itemCount;
    int maxItems;
} UserList;

UserList *createUserList(int maxItems);
bool addUserListItem(UserList* list, User user);
bool removeUserListItem(UserList* list, const char* uuid);
void freeUserList(UserList* list);
User* getUserAtIndex(UserList* list, int index);
bool searchUserListContainsUUID(UserList* list, const char* uuid);
void printUserList(UserList *list);

#endif // USERLIST_H_
