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
    BoundedBuffer* buffer;
    int numMsg;
    int id;
} Producer;


// Dispatcher struct:
typedef struct {
    BoundedBuffer** producerBuffers; //has access
    int numProducers;
    BoundedBuffer** coEditorBuffers; //has access
} Dispatcher;


// Co-Editor struct:
typedef struct {
    BoundedBuffer* buffer;
    int id;
    UnboundedBuffer* screenBuffer; //has access
} CoEditor;


// Screen Manager struct:
typedef struct {
    UnboundedBuffer* buffer;
} ScreenManager;


//Jobs:
void* produce(void* arg);
void* dispatch(void* arg);
void* edit(void* arg);
void* printer(void* arg);

#endif
