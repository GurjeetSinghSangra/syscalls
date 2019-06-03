#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../inc/errExit.h"
#include "../inc/sharedmemory.h"
#include "../inc/semaphore.h"
#include "../inc/constant.h"

const int SERVICE_PRINT = 6;
const int SERVICE_SAVE = 7;
const int SERVICE_SEND = 8;

const char *IPC_SHD_MEM_KEY_PATH = "../IPC_KEYS/ipc_key_mem.conf";
const char *IPC_SEM_KEY_PATH = "../IPC_KEYS/ipc_key_sem.conf";

int main (int argc, char *argv[]) {
    if(argc < 3) {
        errExit("Wrong arugments number, minimum 3 please...");
    }

    char *userCode = argv[1];
    long key = strtol(argv[2], NULL, 10);

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
    enterInCriticalSection(semid);
    int findMark = findAndMark(userCode, key, pointer);
    exitFromCriticalSection(semid);
    
    freeSharedMemory(pointer);
    
    //IF key found
    if(findMark > 0) {
        printf("Chiave trovata e rimossa dalla memoria!\n");
        //TODO change to 3
        int length = argc-2;
        char *args[length+1];
        int j = 3;
        for(int i=1; i<length; i++) {
            args[i] = argv[j];
            j++;
        }
        args[length] = (char *)NULL;
        //TODO recognize the service from the key...
        int service = (int) (key % 10);
        if(service == SERVICE_PRINT) {
            args[0] = "stampa";
            execvp("./stampa", args);
        } else if(service == SERVICE_SEND) {
            args[0] = "invia";
            execvp("./invia", args);
        } else if(service == SERVICE_SAVE) {
            args[0] = "salva";
            execvp("./salva", args);
        } else {
            printf("Servizio inesistente. Errore con la chiave data...\n");
        }
        errExit("erro executing program");
    } else {
        if(findMark == -1) {
            printf("La chiave richiesta è già stata utilizzata!\n");
        } else {
            printf("Coppia chiave, utente ( %li, %s) inesistente.\n", key, userCode);
        }
    }
    
    return 0;
}
