#include "clientqueue.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

ClientQueue* createClientQueue() {
    ClientQueue* q = (ClientQueue*)malloc(sizeof(Node));
    if (!q) return NULL;
    q->front = q->rear = NULL;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
    return q;
}

void ClientEnqueue(ClientQueue* q, const char* str) {
    pthread_mutex_lock(&q->lock);

    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        pthread_mutex_unlock(&q->lock);
        return;
    }

    newNode->data = (char*)malloc(strlen(str) + 1); // +1 for null terminator
    if (!newNode->data) {
        free(newNode);
        pthread_mutex_unlock(&q->lock);
        return;
    }
    strcpy(newNode->data, str);
    newNode->next = NULL;

    // Special case if queue is empty
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }

    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
}

// REMEMBER TO FREE STRING AFTER USE
char* ClientDequeue(ClientQueue* q) {
    pthread_mutex_lock(&q->lock);

    /* if (q->front == NULL) { */
    /*     pthread_mutex_unlock(&q->lock); */
    /*     return NULL; //Queue is empty */
    /* } */
    while (q->front == NULL) {
        pthread_cond_wait(&q->cond, &q->lock);
    }

    Node* temp = q->front;
    char* data = temp->data;
    q->front = q->front->next;

    // If front is null, the rear must also be null
    if (q->front == NULL)
        q->rear = NULL;

    free(temp); // DOES NOT FREE STRING
    pthread_mutex_unlock(&q->lock);
    return data;
}

char* ClientPeek(ClientQueue* q) {
    pthread_mutex_lock(&q->lock);

    if (q->front == NULL) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }

    char* data = q->front->data;

    pthread_mutex_unlock(&q->lock);
    return data;
}

void freeClientQueue(ClientQueue *q) {
    if (q == NULL) return;

    pthread_mutex_lock(&q->lock);

    // Start from the front, free everything
    Node* current = q->front;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }

    pthread_mutex_unlock(&q->lock);
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->cond);

    free(q);
}
