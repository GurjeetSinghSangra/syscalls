#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h> //pid_t type
#include <sys/stat.h> //used for fifo creation
#include <fcntl.h> //for file descriptor
#include <unistd.h>

#include "../inc/constant.h"
#include "../inc/errExit.h"
#include "../inc/request_response.h"

char *baseClientFifoPath = "/tmp/client_fifo";
char *serverFifoPath = "/tmp/server_fifo";

int main (int argc, char *argv[]) {
    //char userCode[USER_CODE_LENGTH];
    //char serviceInput[USER_CODE_LENGTH];
    char clientFifoPath [100];
    
    struct Request request;
    request.pid = getpid();

    printf("Benvenuto a clientReq!\n");
    printf("Inserire codice identificativo: ");
    scanf("%s", request.user_code);
    
    printf("Inserire il servizio richiesto: ");
    scanf("%s", request.service);

    //FIFO CLIENT CREATION
    sprintf(clientFifoPath, "%s_%d", baseClientFifoPath, getpid());//maybe to change sprintf con concat
    
    if(mkfifo(clientFifoPath, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
        errExit("Creation fifo client failed");
    
    //Connection to server FIFO
    int serverFifoFD = open(serverFifoPath, O_WRONLY);
    if(serverFifoFD == -1) {
        errExit("Connecting to server FIFO has failed");
    }
    //Send data to Server thru FIFO SERVER
    //memcpy(request.service, serviceInput, sizeof(serviceInput));
    if(write(serverFifoFD, &request, sizeof(struct Request)) 
        != sizeof(struct Request)) {
        errExit("Write Request in FIFO Server has failed");
    }

    //Wait and read for response in FIFO client
    printf("Messaggio inviato, attesa di risposta... \n");
    int clientFifoPathFD = open(clientFifoPath, O_RDONLY);
    if(clientFifoPathFD == -1) {
        errExit("Read client FIFO has failed");
    }

    struct Response response;
    int bufferRead = -1;
    bufferRead = read(clientFifoPathFD, &response, sizeof(struct Response));
    if(bufferRead != sizeof(struct Response)) {
        errExit("Response Fifo Client read failed");
    }
    if(response.noservice) {
        printf("Servizio richiesto al server non disponibile\n");
    } else {
        printf("Chiave rilasciata dal server: %li \n", response.key);
    }
    //Close and unlink the FD and file
    if(close(serverFifoFD) || close(clientFifoPathFD) == -1)
        errExit("Error closing FIFOs");
    
    if(unlink(clientFifoPath) != 0){
        errExit("Unlink client fifo has failed");
    }
    
    return 0;
}
