#include "actors.h"


//The producers job: to produce types of news and add them to their buffers
void* produce(void* arg) {
    Producer* producer = (Producer*)arg;
    int sportsCount = 0, newsCount = 0, weatherCount = 0;
    for (int i = 0; i < producer->numMsg; i++) {
        usleep(50000);
        char message[100];
        // Generate a random number between 1 and 3
        int randomNumber = rand() % 3 + 1;
        // Create the message according to the number:
        switch (randomNumber) {
            case 1:
                snprintf(message, sizeof(message), "Producer %d SPORTS %d", producer->id, sportsCount);
                sportsCount++;
                break;
            case 2:
                snprintf(message, sizeof(message), "Producer %d NEWS %d", producer->id, newsCount);
                newsCount++;
                break;
            case 3:
                snprintf(message, sizeof(message), "Producer %d WEATHER %d", producer->id, weatherCount);
                weatherCount++;;
                break;
        }
        pushBoundedBuffer(producer->buffer, message); // Insert the message into the bounded buffer
    }
    char* doneMessage = "Done";
    pushBoundedBuffer(producer->buffer, doneMessage); // Insert the done message into the bounded buffer
    return NULL;
}

//The dispatchers job: to dispatch all of producers products to the coeditors according to type of news, in RR fashion.
void* dispatch(void* arg) {
    Dispatcher* dispatcher = (Dispatcher*)arg;
    int allDone = 0;
    while (true) { // go over producer buffers in RR
        if (allDone == dispatcher->numProducers) { //check if done
            char* doneMessage = "Done";
            for(int i=0; i < 3; i++){ //send done message to the co-editors
                pushUnboundedBuffer(dispatcher->coEditorBuffers[i], doneMessage);
            }
            break;
        }
        for (int i = 0; i < dispatcher->numProducers; i++) { //check each buffer in its turn and check if its empty
            BoundedBuffer* producerBuffer = dispatcher->producerBuffers[i];
            if (isBoundedBufferEmpty(producerBuffer)) {
                continue; // skip for next turn
            }
            // if not empty, remove a message from the bounded buffer
            char* message = popBoundedBuffer(producerBuffer);
            if (strcmp(message, "Done") == 0) {
                allDone++;
            } else {
                //getting the type of news (the third token):
                char* temp = strdup(message);
                char* type = NULL;
                char* token = strtok(temp, " ");
                int spaces = 0;
                while (token != NULL) {
                    spaces++;
                    if (spaces == 3){
                        type = strdup(token);
                        break;
                    }
                    token = strtok(NULL, " ");
                }
                if (type != NULL) {
                    if (strcmp(type, "SPORTS") == 0) {
                        pushUnboundedBuffer(dispatcher->coEditorBuffers[0], message);
                    } else if (strcmp(type, "NEWS") == 0) {
                        pushUnboundedBuffer(dispatcher->coEditorBuffers[1], message);
                    } else if (strcmp(type, "WEATHER") == 0) {
                        pushUnboundedBuffer(dispatcher->coEditorBuffers[2], message);
                    }
                }
                free(temp);
                free(type);
                free(message);
            }
        }
    }
    return NULL;
}


//The coeditors jobs: to "edit" each news by blocking for a short time and pushing them to the screen buffer.
void* edit(void* arg) {
    CoEditor* coEd = (CoEditor*)arg;
    while (true) {
        // Pop a message from the unbounded buffer
        if (isUnboundedBufferEmpty(coEd->buffer)){
            continue;
        }
        char* message = popUnboundedBuffer(coEd->buffer);
        if (strcmp(message, "Done") == 0) {  // Exit the loop if the message is "Done"
            pushBoundedBuffer(coEd->screenBuffer, message); // push without waiting
            free(message);
            break; //no more messages of this type
        }
        // Block for 0.1 second:
        usleep(100000);
        // Push the message to the screen manager unbounded buffer
        pushBoundedBuffer(coEd->screenBuffer, message);
        free(message);
    }
    return NULL;
}


//The screen managers job: to take each message out of the screen buffer and print it to the screen.
void* printer(void* arg) {
    ScreenManager* screenManager = (ScreenManager*)arg;
    int doneCount = 0;
    while (true) {
        // Pop a message from the unbounded buffer:
        char* message = popBoundedBuffer(screenManager->buffer);
        // Print the message to screen
        if (strcmp(message, "Done") != 0) {
            printf("%s\n", message);
            free(message);
        } else {
            doneCount++;
            free(message);
            if (doneCount == 3) { // after three "Done"'s, exit
                char* doneMessage = "DONE";
                printf("%s\n", doneMessage);
                break;
            }
        }
    }
    return NULL;
}
