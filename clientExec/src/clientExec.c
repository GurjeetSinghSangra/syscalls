#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"
#include "../inc/semaphore.h"
#include "../inc/constant.h"


const char *IPC_SHD_MEM_KEY_PATH = "../IPC_KEYS/ipc_key_mem.conf";
const char *IPC_SEM_KEY_PATH = "../IPC_KEYS/ipc_key_sem.conf";
const char *STAMPA = "Stampa";
const char *INVIA = "Invia";
const char *SALVA = "Salva";

int main (int argc, char *argv[]) {
    if(argc < 4) {
        errExit("Wrong arugments number, minimum 3 please...");
    }

    char *userCode = argv[1];
    long key = strtol(argv[2], NULL, 10);
    char *serviceInput = argv[3];

    //--SEMAPHORE--
    key_t semkey = ftok(IPC_SEM_KEY_PATH, 'a');
    if(semkey == -1) {
        errExit("Creation token for semaphore failed");
    }
    int semid = getSemaphore(semkey, 1);

    //----MEMORIA CONDIVISA----
    //ACCEDI ALLA MEMORIA CONDIVISA, Attach, utilizzo dei semafori
    key_t memkey = ftok(IPC_SHD_MEM_KEY_PATH, 'a');
    if(memkey == -1) {
        errExit("Creation token for memory failed");
    }
    int shmid = getSharedMemory(memkey, sizeof(struct Memoryrow) * (int) LENGTH_SHARED_MEM);
    struct Memoryrow *pointer = (struct Memoryrow*) attachSharedMemory(shmid, 0);
    
    //find and mark key
    enterInCriticalSection(semid, 0);
    int findMark = findAndMark(userCode, key, pointer);
    exitFromCriticalSection(semid, 0);
    
    freeSharedMemory(pointer);
    
    //TODO: retrive the service from the key

    //IF key found
    if(findMark > 0) {
        printf("Chiave trovata!\n");
        //TODO change to 3
        int length = argc-4;
        char *args[length+1];
        int j = 4;
        for(int i=0; i<length; i++) {
            args[i] = argv[j];
            printf("%s\n", args[i]);
        }
        args[length] = (char *)NULL;
        //TODO recognize the service from the key...
        if(strcmp(serviceInput, STAMPA) == 0) {
            execvp("stampa", args);
        } else if(strcmp(serviceInput, INVIA) == 0) {
            execvp("invia", args);
        } else if(strcmp(serviceInput, SALVA) == 0) {
            execvp("salva", args);
        } else {
            printf("Servizio inesistente.\n");
        }
    } else {
        if(findMark == -1) {
            printf("La chiave richiesta è già stata utilizzata!\n");
        } else {
            printf("coppia chiave, utente ( %li, %s) inesistente.\n", key, userCode);
        }
    }
    
    return 0;
}
