#ifndef _SEMAPHORE_HH
#define _SEMAPHORE_HH

#include <stdlib.h>
#include <sys/sem.h>

// definition of the union semun
union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void semOp (int semid, unsigned short semaphoreNumb, short operation);
int createSemaphore(key_t key, int numsem);
int removeSemaphore(int semid);
int enterInCriticalSection(int semId, unsigned short semaphoreNumb);
int exitFromCriticalSection(int semId, unsigned short semaphoreNumb);

#endif