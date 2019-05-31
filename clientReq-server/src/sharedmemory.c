#include <sys/shm.h>
#include <sys/stat.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"

//Create shared memory
int createSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR);
    if(shmid == -1) {
        errExit("Creation memory failed");
    }
    return shmid;
}

//get shared memory in case that the shared memory is already in place return -1
int getSharedMemory(key_t key, size_t size) {
    int shmid = shmget(key, size, IPC_CREAT | S_IWGRP | S_IRGRP | S_IRUSR | S_IWUSR);
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

void removeSharedMemory(int shimd) {
    if(shmctl(shimd, IPC_RMID, NULL) == -1)
        errExit("Remove shared memory failed");
}