#ifndef ACTORS_H
#define ACTORS_H
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include "buffers.h"

// Here we have all the actor objects and their jobs.


// Producer struct:
typedef struct {
    int id;
    int numMsg;
    BoundedBuffer* buffer;
} Producer;


// Dispatcher struct:
typedef struct {
    int numProducers;
    BoundedBuffer** producerBuffers; //has access
    UnboundedBuffer** coEditorBuffers; //has access
} Dispatcher;


// Co-Editor struct:
typedef struct {
    int id;
    UnboundedBuffer* buffer;
    BoundedBuffer* screenBuffer; //has access
} CoEditor;


// Screen Manager struct:
typedef struct {
    BoundedBuffer* buffer;
} ScreenManager;


//Jobs:
void* produce(void* arg);

void* dispatch(void* arg);

void* edit(void* arg);

void* printer(void* arg);

#endif
