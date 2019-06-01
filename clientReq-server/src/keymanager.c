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
struct Memoryrow *mempointer;//do we need the pointer?
int lastInsertedKeyPos = 0;
const int TIME_THRESHOLD = 60 * 5;

void alarmHandler(int sig) {
    if(sig == SIGALRM) {
        //DELETE KEYs TASK
        printf("Alarm called after 30 sec! My pid: %d shmid: %d semid: %d\n", getpid(), shmid, semid);
        enterInCriticalSection(semid, 0);
        deleteDeprecatedKeys();
        exitFromCriticalSection(semid, 0);
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
    printf("Insterting key in memory\n");
    enterInCriticalSection(semid, 0);
    if(lastInsertedKeyPos < LENGTH_SHARED_MEM) {
        struct Memoryrow newRow;
        newRow.key = key;
        strcpy(newRow.userCode, userCode);
        newRow.timestamp = time(NULL);
        printf("Inserting user with code: %s\n", newRow.userCode);
        mempointer[lastInsertedKeyPos] = newRow;
        lastInsertedKeyPos++;
        struct Memoryrow row = mempointer[(lastInsertedKeyPos -1)];
        printf("Value has been inserted %s, %ld at position %d\n", row.userCode, row.key, lastInsertedKeyPos -1);
        return 1;
    } else {
        //The memory is full!!!
        //Could do possible that the keymanager task has deleted some values
        //DELETED values, are values with key initiliazed to 0
        //Let overwrite them, thus it will represent the new value
        int foundSlot = 0;
        for(int i=0; i<LENGTH_SHARED_MEM && foundSlot == 0; i++) {
            struct Memoryrow *row = &(mempointer[i]);
            if(row->key ==0) {
                strcpy(row->userCode, userCode);
                row->timestamp = time(NULL);
                row->key = key;
                foundSlot = 1;
            }
        }
    }
    //TODO exit from critical section
    exitFromCriticalSection(semid, 0);
    return 0;
}

void deleteDeprecatedKeys() {
    time_t timestamp = time(NULL);
    for(int i=0; i<LENGTH_SHARED_MEM && i<=lastInsertedKeyPos; i++) {
        struct Memoryrow *row = &(mempointer[i]);
        //filter all data which are NOT DELETED and are OLD
        if(row->key != 0 && (timestamp - row->timestamp) > TIME_THRESHOLD) {
            row->key=0;
        }
    }
}