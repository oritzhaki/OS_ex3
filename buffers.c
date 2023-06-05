#include "buffers.h"

//there are two types of buffers: bounded and unbounded. both structs, unbounded is a linked list

// initializes the bounded queue with a given size
void initBoundedBuffer(BoundedBuffer* buffer, int size) {
    buffer->buffer = (char**)malloc(size * sizeof(char*));
    buffer->front = 0;
    buffer->rear = 0;
    buffer->size = size;
    // init semaphores:
    sem_init(&buffer->mutex, 0, 1);
    sem_init(&buffer->fullSlots, 0, 0);
    sem_init(&buffer->emptySlots, 0, size);

//    int mutexValue, fullSlotsValue, emptySlotsValue;
//    sem_getvalue(&buffer->mutex, &mutexValue);
//    sem_getvalue(&buffer->fullSlots, &fullSlotsValue);
//    sem_getvalue(&buffer->fullSlots, &emptySlotsValue);
//
//    printf("after init - Value of buffer->mutex: %d\n", mutexValue);
//    printf("after init - Value of buffer->fullSlots: %d\n", fullSlotsValue);
//    printf("after init - Value of buffer->fullSlots: %d\n", emptySlotsValue);
//    printf("size is %d\n", size);
}

// initializes unbounded queue
void initUnboundedBuffer(UnboundedBuffer* buffer) {
    buffer->head = NULL; //empty linked list
    buffer->tail = NULL;
    sem_init(&buffer->mutex, 0, 1);
    sem_init(&buffer->fullSlots, 0, 0);
}

// pushes an element into the end of the queue (FIFO) if there is room
void pushBoundedBuffer(BoundedBuffer* buffer, char* s) {
    sem_wait(&buffer->emptySlots); //ensures that the consumer waits if the buffer is full.
    sem_wait(&buffer->mutex); //protect critical sections of the code

    buffer->buffer[buffer->rear] = strdup(s);
    buffer->rear = (buffer->rear + 1) % buffer->size; //cyclic

    sem_post(&buffer->mutex);
    sem_post(&buffer->fullSlots);
}

// pops an element from the front of the bounded queue (FIFO) if not empty
char* popBoundedBuffer(BoundedBuffer* buffer) {
    sem_wait(&buffer->fullSlots); //ensures that the consumer waits if the buffer is empty.
    sem_wait(&buffer->mutex); //protect critical sections of the code

    char* item = buffer->buffer[buffer->front]; // get the item in the front of the buffer - FIFO
    buffer->front = (buffer->front + 1) % buffer->size;

    sem_post(&buffer->mutex);
    sem_post(&buffer->emptySlots);
    return item;
}

// pops an element from the start of the unbounded queue (FIFO)
char* popUnboundedBuffer(UnboundedBuffer* buffer) {
    sem_wait(&buffer->fullSlots); //ensures that the consumer waits if the buffer is empty.
    sem_wait(&buffer->mutex); //protect critical sections of the code

    Node* nodeToRemove = buffer->head;
    char* item = nodeToRemove->data;
    buffer->head = buffer->head->next;
    if (buffer->head == NULL) {
        buffer->tail = NULL;
    }
    free(nodeToRemove);

    sem_post(&buffer->mutex);
    return item;
}

// Destroys semaphores an frees memory
void destroyBoundedBuffer(BoundedBuffer* buffer) {
    sem_destroy(&buffer->mutex);
    sem_destroy(&buffer->fullSlots);
    sem_destroy(&buffer->emptySlots);
    free(buffer->buffer);
}

// pushes an element into the end of the unbounded queue (FIFO)
void pushUnboundedBuffer(UnboundedBuffer* buffer, char* s) {
    Node* newNode = (Node*)malloc(sizeof(Node)); //each element is allocated memory
    newNode->data = strdup(s);
    newNode->next = NULL;

    sem_wait(&buffer->mutex); //protect critical sections of the code

    if (buffer->tail == NULL) {
        buffer->head = newNode;
        buffer->tail = newNode;
    } else {
        buffer->tail->next = newNode;
        buffer->tail = newNode;
    }

    sem_post(&buffer->fullSlots); //not empty
    sem_post(&buffer->mutex);
}

// checks if the bounded queue is empty
int isBoundedBufferEmpty(BoundedBuffer* buffer) {
    int isEmpty;
    sem_wait(&buffer->mutex); //protect critical sections of the code
    isEmpty = (buffer->front == buffer->rear);
    sem_post(&buffer->mutex);
    return isEmpty;
}

//checks if unbounded buffer is empty
bool isUnboundedBufferEmpty(UnboundedBuffer* buffer) {
    sem_wait(&buffer->mutex); //protect critical sections of the code
    bool empty = (buffer->head == NULL);
    sem_post(&buffer->mutex);
    return empty;
}

//destroys semaphores and frees memory
void destroyUnboundedBuffer(UnboundedBuffer* buffer) {
    sem_destroy(&buffer->mutex);
    sem_destroy(&buffer->fullSlots);

    Node* current = buffer->head;
    while (current != NULL) {
        Node* next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}
