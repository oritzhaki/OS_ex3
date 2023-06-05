#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffers.h"
#include "actors.h"


int runNewsFlow(int* configurationArr, int length){
    // create producers with bounded buffers and send each producer to work:
    int numProducers = (length - 1)/3;
    Producer* producers[numProducers]; // array of n producers
    BoundedBuffer* producerBuffersArr[numProducers]; // array of n producer buffers
    pthread_t producersThreads[numProducers]; // array of n producer threads
    for (int i = 0; i < numProducers; i++) {
        // Get the data for each producer:
        int id = configurationArr[i * 3] - 1;
        int numItems = configurationArr[i * 3 + 1];
        int bufferSize = configurationArr[i * 3 + 2];
        BoundedBuffer* buff = malloc(sizeof(BoundedBuffer));
        if (buff == NULL) {
            perror("Error in: Memory allocation.\n");
            return -1;
        }
        initBoundedBuffer(buff, bufferSize);
        producerBuffersArr[i] = buff;
        Producer* prod = malloc(sizeof(Producer));
        if (prod == NULL) {
            perror("Error in: Memory allocation.\n");
            return -1;
        }
        // Init producer:
        prod->buffer = buff;
        prod->numMsg = numItems;
        prod->id = id;
        producers[i] = prod;
        // Send producer to work:
        pthread_create(&producersThreads[i], NULL, produce, (void*)prod);
    }

    // Create screen manager with a bounded queue:
    int sMbufferSize = configurationArr[length - 1]; // last in arr
    BoundedBuffer screenManagerBuffer;
    initBoundedBuffer(&screenManagerBuffer, sMbufferSize);
    ScreenManager ScreenManager = {&screenManagerBuffer};

    // Create co-editors with unbounded queues:
    UnboundedBuffer coEditorBuffer1; //S
    initUnboundedBuffer(&coEditorBuffer1);
    CoEditor coEditor1 = {&coEditorBuffer1, 1, &screenManagerBuffer};
    UnboundedBuffer coEditorBuffer2; //N
    initUnboundedBuffer(&coEditorBuffer2);
    CoEditor coEditor2 = {&coEditorBuffer2, 2, &screenManagerBuffer};
    UnboundedBuffer coEditorBuffer3; //W
    initUnboundedBuffer(&coEditorBuffer3);
    CoEditor coEditor3 = {&coEditorBuffer3, 3, &screenManagerBuffer};
    // create an array of the co-editor buffers:
    UnboundedBuffer* coEditorBuffersArr[3];
    coEditorBuffersArr[0] = &coEditorBuffer1;
    coEditorBuffersArr[1] = &coEditorBuffer2;
    coEditorBuffersArr[2] = &coEditorBuffer3;

    //Create the dispatcher and send to work:
    Dispatcher dispatcher;
    dispatcher.numProducers = numProducers;
    dispatcher.producerBuffers = producerBuffersArr; //has access
    dispatcher.coEditorBuffers = coEditorBuffersArr; //has access
    pthread_t dispatcherThread;
    pthread_create(&dispatcherThread, NULL, dispatch, (void*)&dispatcher);

    // send co-editors to work:
    pthread_t coEditorT1;
    pthread_t coEditorT2;
    pthread_t coEditorT3;
    pthread_create(&coEditorT1, NULL, edit, (void*)&coEditor1);
    pthread_create(&coEditorT2, NULL, edit, (void*)&coEditor2);
    pthread_create(&coEditorT3, NULL, edit, (void*)&coEditor3);

    // send screen manager to work:
    pthread_t screenManagerThread;
    pthread_create(&screenManagerThread, NULL, printer, (void*)&ScreenManager);

    // wait for all threads to finish, distroy all semaphores and free alocated memory:
    pthread_join(dispatcherThread, NULL);  // Wait for dispatcher
    for (int i = 0; i < numProducers; i++) {  // wait for producers
        pthread_join(producersThreads[i], NULL);
        destroyBoundedBuffer(producerBuffersArr[i]);
        free(producerBuffersArr[i]);
        free(producers[i]);
    }
    // Wait for co-editors:
    pthread_join(coEditorT1, NULL);
    pthread_join(coEditorT2, NULL);
    pthread_join(coEditorT3, NULL);
    pthread_join(screenManagerThread, NULL);  // Wait for manager thread to finish
    for (int i = 0; i < 3 ; i++) {
        destroyUnboundedBuffer(coEditorBuffersArr[i]);
    }
    destroyBoundedBuffer(&screenManagerBuffer);
    free(configurationArr);
    return 0;
}


// counts the number of rows in the conf file with data
int getDataLength(const char* path){
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("Error in: fopen\n");
        return NULL;
    }
    // count the rows with the information (producers + co-editor):
    int counter = 0;
    int ch;
    int enters = 0;
    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            enters++;
            if (enters % 4 == 0) { // every four rows there is an empty row so dont count
                continue;
            }
            counter++;
        }
    }
    counter++; // the last row doesnt end with an enter but still should count it
    return counter;
}


// prepare an array of the configuration data for convenience:
int* createDataArr(const char* path, int counter) {
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        perror("Error in: fopen\n");
        return NULL;
    }
    // create the array of size counter with allocated memory
    int* valArr = (int*)malloc(counter * sizeof(int));
    if (valArr == NULL) {
        perror("Error in: malloc");
        fclose(file);
        return NULL;
    }
    int i = 0;
    char line[100]; // max size of line
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] != '\n') {  // Skip empty lines
            int value = atoi(line); // assuming all rows are ints
            valArr[i] = value;
            i++;
        }
    }
    fclose(file);
    return valArr;
}


//program will receive the name of the configuration file from the command line.
int main(int argc, char* argv[]) {
    //check num arguments
    if (argc != 2) {
        printf("Invalid argument\n");
        return -1;
    }

    // read configuration file and save data in array:
    const char* filePath = argv[1];
    int length = getDataLength(filePath);
    int* configurationArr = createDataArr(filePath, length);
    if (configurationArr == NULL) {
        return -1;
    }

    //start sending the news:
    int status = runNewsFlow(configurationArr, length);

    return status;
}
