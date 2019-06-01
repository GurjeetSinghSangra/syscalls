#ifndef _SHAREDMEMORY_HH
#define _SHAREDMEMORY_HH

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include "constant.h"

struct Memoryrow {
    char userCode[USER_CODE_LENGTH];
    long key;
    time_t timestamp;
};

//Create shared memory
int createSharedMemory(key_t key, size_t size);

//get shared memory in case that the shared memory is already in place return -1
int getSharedMemory(key_t key, size_t size);

//attach the shared memory
void *attachSharedMemory(int shmid, int shmflg);

//detach
void freeSharedMemory(void *ptrSharedMemory);

int findAndMark(char *usercode, long key, struct Memoryrow *ptrSharedMemory);

#endif