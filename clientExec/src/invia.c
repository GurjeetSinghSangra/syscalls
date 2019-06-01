#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "../inc/errExit.h"

struct Message {
    long type; /* Message type */
    char *mtext; /* Message body */
};

int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma Invia!\n");
    if(argc < 3)
        errExit("Invalid arguments, minimum 3");
    int msgkey = atoi(argv[0]);
    if(msgkey < 0) {
        errExit("Invalid key");
    }
    int msgid = msgget(msgkey,  IPC_CREAT | S_IRUSR | S_IWUSR);
    if(msgid == -1) {
        errExit("Error open queue");
    }
    
    struct Message message;
    //calculate the size of the string
    int length = 0;
    for(int i=2; i<argc; i++) {
        length += strlen(argv[i]);
        length++; //for the space character
    }

    message.mtext = (char *)malloc(sizeof(char) * length);
    for(int i=2; i<argc; i++) {
        strcat(message.mtext, argv[i]);
        strcat(message.mtext, " ");
    }
    message.mtext[length-1] = '\0';
    message.type = 1;

    if(msgsnd(msgid, &message, sizeof(message) - sizeof(long), 0) == -1)
        errExit("Error sending message");
    printf("Messaggio inviato: %s\nFine...\n", message.mtext);
    return 0;
}

