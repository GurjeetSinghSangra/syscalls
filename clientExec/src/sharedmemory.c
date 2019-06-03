#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"

//get shared memory in case that the shared memory is already in place return -1
int getSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR);
    if(shmid == -1) {
        //chiamato dai processi esterni controllare se la tabella esiste gia, gli altri processi non devono creare la memoria ma solo ottenerla
        errExit("Creation memory failed");
    }
    return shmid;
}

//attach the shared memory
void *attachSharedMemory(int shmid, int shmflg) {
    void *pointer = shmat(shmid, NULL, shmflg);
    if (pointer == (void *)-1)
        errExit("Attach shared memory failed");
    return pointer;
}

//detach
void freeSharedMemory(void *ptrSharedMemory) {
    if(shmdt(ptrSharedMemory) == -1)
        errExit("Dettach shared memory failed");
}

//Remember items with usercode empty and key with value >0 are used keys.
//If item exists, mark it used and return 1.
//if item is already used, it returns -1.
//If does not exists return 0.
int findAndMark(char *usercode, long key, struct Memoryrow *ptrSharedMemory) {
    for(int i=0; i< LENGTH_SHARED_MEM; i++) {
        struct Memoryrow *row = &(ptrSharedMemory[i]);
        //Not exists
        //Deleted key not found, or (key == 0 and code == usercode)
        if(row->key == key) {
            if(strcmp(row->userCode, usercode) == 0) {
                if(row->key == key) {
                    strcpy(row->userCode, "");
                    return 1;
                }
            } else {
                if(strcmp(row->userCode, "") == 0) {
                    return -1;
                }
            }
        }
    }
    return 0;
}