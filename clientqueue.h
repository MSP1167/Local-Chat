#ifndef CLIENTQUEUE_H_
#define CLIENTQUEUE_H_
#include <pthread.h>

typedef struct Node {
    char* data;
    struct Node* next;
} Node;

typedef struct ClientQueue {
    Node* front;
    Node* rear;
    pthread_mutex_t lock;
    pthread_cond_t  cond;
} ClientQueue;

ClientQueue* createClientQueue();

void ClientEnqueue(ClientQueue* q, const char* data);

char* ClientDequeue(ClientQueue* q);

char* ClientPeek(ClientQueue* q);

void freeClientQueue(ClientQueue *q);

#endif // CLIENTQUEUE_H_
