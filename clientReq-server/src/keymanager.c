#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h> //for file descriptor, servira?
#include <string.h>

#include "../inc/keymanager.h"
#include "../inc/errExit.h"
#include "../inc/semaphore.h"

int shmid;
int semid;
struct Memoryrow *mempointer;
int *maxRowUsed;
const int TIME_THRESHOLD = 60 * 5;

void alarmHandler(int sig) {
    if(sig == SIGALRM) {
        //DELETE KEYs TASK
        printf("<Key Manager: %d> Alarm triggered!\n", getpid());
        enterInCriticalSection(semid);
        deleteDeprecatedKeys();
        exitFromCriticalSection(semid);
    }
}

int keymanager(int memoryId, int semaphoreId, struct Memoryrow *pointer) {
    shmid = memoryId;
    semid = semaphoreId;
    mempointer = pointer;

    if(signal(SIGALRM, alarmHandler) == SIG_ERR)
        errExit("Alarm handler failed");

    while(1) {
        alarm(30);
        pause();
    }
    
    return 0;
}

//Caled just by the server (parent) process
int insertKey(long key, char userCode[]) {
    //TODO: ENTER in critical section
    enterInCriticalSection(semid);
    //find first slot
    int foundSlot = 0;
    for(int i=0; i<LENGTH_SHARED_MEM && foundSlot == 0; i++) {
        struct Memoryrow *row = &(mempointer[i]);
        if(row->key == 0) {
            strcpy(row->userCode, userCode);
            row->timestamp = time(NULL);
            row->key = key;
            printf("Row inserted in memory at pos %i\n", i);
            foundSlot = 1;
            if((*maxRowUsed) <= i) {
                (*maxRowUsed)++;
            }
        }
    }
    //TODO exit from critical section
    exitFromCriticalSection(semid);
    return foundSlot;
}

//CALLED Just by the server process
//DELETED rows, are items with and empty usercode and key equal to 0
//Remember items with usercode empty and key with value >0 are used keys.
void deleteDeprecatedKeys() {
    time_t timestamp = time(NULL);
    for(int i=0; i <(*maxRowUsed) && i< LENGTH_SHARED_MEM; i++) {
        struct Memoryrow *row = &(mempointer[i]);
        //filter all data which are NOT DELETED and are OLD
        int interval = (int) (timestamp - row->timestamp);
        if(row->key != 0 && interval > (int) (TIME_THRESHOLD)) {
            printf("Row deleted at pos %i, usercode: %s key: %li\n", i, row->userCode, row->key);
            row->key=0;
            strcpy(row->userCode, "");
        }
    }
}