#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include "../inc/errExit.h"
#include "../inc/keymanager.h"
#include "../inc/request_response.h"
#include "../inc/keygenerator.h"

const int NO_SERVICE = 4000;
const int SERVICE_PRINT = 3000;
const int SERVICE_SAVE = 2000;
const int SERVICE_SEND = 1000;

char *serverFifoPath = "/tmp/server_fifo";
char *baseClientFifoPath = "/tmp/client_fifo";
pid_t childPid;
int serverFifoFD;
int serverFifoExtraFD;
long requestNumber = 1;

void quit() {
    //Exit be sure to close every resources: server fifo, memory, semaphores.
    if (serverFifoFD != 0 && close(serverFifoFD) == -1)
            errExit("close failed");

    if (serverFifoExtraFD != 0 && close(serverFifoExtraFD) == -1)
        errExit("close failed");

    if (unlink(serverFifoPath) != 0)
        errExit("unlink failed");

    _exit(0);
}

void sigHandler(int sig) {
    if(sig == SIGINT && childPid != 0) {
        kill(childPid, SIGKILL);//what if the keymanager is using the memeory during the kill
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

    printf("Processo server partito!\n");

    //-----SIGNALS-----
    //Mask signals except SigInt
    sigset_t signalSet;
    if(sigfillset(&signalSet) == -1)
        errExit("Error filling signal set");
    if(sigdelset(&signalSet, SIGINT) == -1)
        errExit("Error removing signal from mask set");
    if(sigprocmask(SIG_SETMASK, &signalSet, NULL) == -1)
        errExit("Error setting mask");
    //Creating and setting signal handler
    if(signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("Error creation signal handler");
    }

    //TODO Create Shared memory

    //TODO create semaphores

    //Create keymanager using fork and start it!
    pid_t pid = fork();
    if(pid == 0) {
        keymanager();
    } else {
        childPid = pid;
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
        
        //Manage clients requests and insert inside the shared memory
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
                printf("Richiesta ricevuta da client %d, codice: %s, servizio: %i\n", request.pid, request.user_code, service);
                if(service != 4000) {
                    //Generation key and insertion
                    long key = generateKey(requestNumber, service);
                    response.key = key;
                    response.noservice = 0;
                    //TODO: Insert in shared Memory
                    insertKey(key, request.user_code);
                    requestNumber++;
                } else {
                    printf("Servizio richiesto non disponibile!\n");
                    response.noservice = 1;
                }
                //Open client FIFO and send resoonse

                //FIFO CLIENT CREATION
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
                        printf("Risposta inviata a client %d key %li, noservice: %d\n", request.pid, response.key, response.noservice);
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
