#ifndef BUFFERS_H
#define BUFFERS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct Node { // for linked list
    char* data;
    struct Node* next;
} Node;

// bounded queue is an array with head, tail, size and personal semaphores to control traffic
typedef struct {
    char** buffer;
    int front;
    int rear;
    int size;
    sem_t mutex;
    sem_t fullSlots;
    sem_t emptySlots;
} BoundedBuffer;

// unbounded queue is a linked list with head, tail and personal semaphores to control traffic
typedef struct {
    Node* head;
    Node* tail;
    sem_t mutex;
    sem_t fullSlots;
} UnboundedBuffer;

void initBoundedBuffer(BoundedBuffer* buffer, int size);

void pushBoundedBuffer(BoundedBuffer* buffer, char* s);

char* popBoundedBuffer(BoundedBuffer* buffer);

void destroyBoundedBuffer(BoundedBuffer* buffer);

int isBoundedBufferEmpty(BoundedBuffer* buffer);

void initUnboundedBuffer(UnboundedBuffer* buffer);

void pushUnboundedBuffer(UnboundedBuffer* buffer, char* s);

char* popUnboundedBuffer(UnboundedBuffer* buffer);

bool isUnboundedBufferEmpty(UnboundedBuffer* buffer);

void destroyUnboundedBuffer(UnboundedBuffer* buffer);

#endif  // BUFFERS_H
