#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ipc.h>

#include "../inc/errExit.h"
#include "../inc/keymanager.h"
#include "../inc/request_response.h"
#include "../inc/keygenerator.h"
#include "../inc/sharedmemory.h"
#include "../inc/semaphore.h"

const int SERVICE_PRINT = 6;
const int SERVICE_SAVE = 7;
const int SERVICE_SEND = 8;
const int NO_SERVICE = 9;

const char *IPC_SHD_MEM_KEY_PATH = "../IPC_KEYS/ipc_key_mem.conf";
const char *IPC_SEM_KEY_PATH = "../IPC_KEYS/ipc_key_sem.conf";

const char *serverFifoPath = "/tmp/server_fifo";
const char *baseClientFifoPath = "/tmp/client_fifo";
sigset_t signalset;
pid_t childpid;
int serverFifoFD;
int serverFifoExtraFD;
long requestNumber = 1;

//
int shmid;
int indexPosShmid;
struct Memoryrow *mempointer;
int *lastFreeCell;

int semid;

void quit() {
    //Close all resources: server fifo, memory, semaphores.
    if (serverFifoFD != 0 && close(serverFifoFD) == -1)
        errExit("close failed");

    if (serverFifoExtraFD != 0 && close(serverFifoExtraFD) == -1)
        errExit("close failed");

    if (unlink(serverFifoPath) != 0)
        errExit("unlink failed");
    
    freeSharedMemory(mempointer);
    removeSharedMemory(shmid);

    freeSharedMemory(lastFreeCell);
    removeSharedMemory(indexPosShmid);
    
    _exit(0);
}

void sigHandler(int sig) {
    printf("Sig term called %d\n", getpid());
    if(sig == SIGTERM) {
        printf("Killing process %d\n", childpid);
        if(kill(childpid, SIGTERM) == -1)
            printf("Killing child task failed!\n");
        wait(NULL);
        quit();
    }
}

const int getService(char serviceInput[]) {
    //la mappatura in interi potrebbe essere sposta sul server
    if(strcmp(serviceInput, "Stampa") == 0) {
        return SERVICE_PRINT;
    } else if(strcmp(serviceInput, "Invia") == 0) {
        return SERVICE_SEND;
    } else if(strcmp(serviceInput, "Salva") == 0) {
        return SERVICE_SAVE;
    }
    return NO_SERVICE;
}

int main (int argc, char *argv[]) {

    printf("Processo server partito con pid: %d!\n", getpid());
    //-----SIGNALS-----
    //Creating and setting signal handler
    if(signal(SIGTERM, sigHandler) == SIG_ERR) {
        errExit("Error creation signal handler");
    }
    //Mask all signals
    if(sigfillset(&signalset) == -1)
        errExit("Error filling signal set");
    if(sigdelset(&signalset, SIGTERM) == -1)
        errExit("Error removing signal from mask set");
    if(sigprocmask(SIG_SETMASK, &signalset, NULL) == -1)
        errExit("Error setting mask");

    //SHARED MEMORY
    indexPosShmid = createSharedMemoryFromSystem(sizeof(int));
    lastFreeCell = attachSharedMemory(indexPosShmid, 0);

    key_t keySharedMem = ftok(IPC_SHD_MEM_KEY_PATH, 'a');
    if(keySharedMem == -1)
        errExit("Ftok for shdmem failed!");
    const int lengthMemory = LENGTH_SHARED_MEM;
    int totalLength = sizeof(struct Memoryrow) * lengthMemory;
    printf("Total length of memory is %d: \n", totalLength);
    shmid = createSharedMemory(keySharedMem, totalLength);
    mempointer = (struct Memoryrow*) attachSharedMemory(shmid, 0);
    printf("This is the shared mem id: %d\n", shmid);

    //SEMAPHORE AND INITIALIZATION
    key_t keySem = ftok(IPC_SEM_KEY_PATH, 'a');
    if(keySem == -1)
        errExit("Ftok for semaphore failed!");
    semid = createSemaphore(keySem, 1);
    printf("This is the semaphore id: %d\n", semid);
    union semun sem;
    sem.val = 1;
    semctl(semid, 0, SETVAL, sem);
    
    //Init child and create keymanager
    pid_t pid = fork();
    if(pid == 0) {
        //CHILD process

        //RESET SIGNAL SIGTERM
        if (signal(SIGTERM, SIG_DFL) == SIG_ERR )
            errExit("Error resetting sigterm for child process");
            
        //Remove the sig alarm from masked signals!
        if(sigdelset(&signalset, SIGALRM) == -1)
            errExit("Error removing alarm from mask set");
        if(sigprocmask(SIG_SETMASK, &signalset, NULL) == -1)
            errExit("Error setting mask for ALRM");
        //start keymanager
        keymanager(shmid, semid, mempointer);
    } else {
        //SERVER parent process code
        childpid = pid;
        //Creation Fifo Server in tmp, the creation could be before, but it should not change anything to the architecture
        if(mkfifo(serverFifoPath, S_IRUSR | S_IWUSR | S_IRGRP) == -1) {
            errExit("Error creation Server FIFO");
        }
        printf("Attesa di un client\n");
        //Open the fifo for read and for extra write to avoid the EOF, remind open is blocking functions!
        serverFifoFD = open(serverFifoPath, O_RDONLY);
        if(serverFifoFD == -1)
            errExit("Reading server fifo Failed");
        
        //Extra fifo writing
        serverFifoExtraFD = open(serverFifoPath, O_WRONLY);
        if(serverFifoExtraFD == -1)
            errExit("Writing server fifo Failed");
        
        //Manage client requests and insert inside the shared memory
        int bufferRead = -1;
        struct Request request;
        do {
            bufferRead = read(serverFifoFD, &request, sizeof(struct Request));
            if(bufferRead != sizeof(struct Request)) {
                printf("Request read failed, incompatible or size differents\n");
            } else {
                struct Response response;
                response.key = -1;
                const int service = getService(request.service);
                printf("Richiesta ricevuta da utente: %s, servizio: %i, pid: %d \n", request.user_code, service, request.pid);
                if(service != NO_SERVICE) {
                    //Generation key and insertion
                    long key = generateKey(requestNumber, request.pid,service);
                    response.key = key;
                    
                    if(insertKey(key, request.user_code) == 0) {
                        printf("Memory is full! Key not inserted\n");
                        response.key = 0; //KEY to zero represents Memory full
                    }
                    requestNumber++;
                } else {
                    printf("Servizio richiesto non disponibile!\n");
                    response.key = -1;
                }
                //Open client FIFO and send resoonse

                //Send response to FIFO CLIENT 
                char clientFifoPath[100];
                sprintf(clientFifoPath, "%s_%d", baseClientFifoPath, request.pid);//maybe to change sprintf con concat
                int clientFifoPathFD = open(clientFifoPath, O_WRONLY);
                if(clientFifoPathFD == -1) {
                    printf("Client FIFO open error!\n");
                } else {
                    if(write(clientFifoPathFD, &response, sizeof(struct Response)) !=
                        sizeof(struct Response)) {
                        printf("Write to client FIFO has failed\n");
                    } else {
                        printf("Risposta inviata a client %s %d key %li\n", request.user_code, request.pid, response.key);
                    }
                }
                if(close(clientFifoPathFD) == -1)
                    printf("Error closing FIFO client\n");
            }
            printf("Attesa di un client\n");
        } while(bufferRead != -1);

    }
    //Exit and close everying, we achieve this position in case of fifo broken
    quit();
}
