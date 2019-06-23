#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "../inc/errExit.h"

#define MAX_SIZE_BUFFER 500

struct Message {
    long type; /* Message type */
    char mtext[MAX_SIZE_BUFFER]; /* Message body */
};

int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma Invia!\n");
    if(argc < 3)
        errExit("Invalid arguments, no list arguments,passed, minimum 3 args");
    int msgkey = atoi(argv[1]);
    if(msgkey < 0) {
        errExit("Invalid key");
    }
    int msgid = msgget(msgkey, S_IRUSR | S_IWUSR);
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

    strcpy(message.mtext, "");
    for(int i=2; i<argc  && strlen(message.mtext) <= MAX_SIZE_BUFFER; i++) {
        if((strlen(message.mtext) + strlen(argv[i])) < MAX_SIZE_BUFFER) {
            strcat(message.mtext, argv[i]);
            strcat(message.mtext, " ");
        }
    }
    message.mtext[strlen(message.mtext)] = '\0';
    message.type = 1;
    size_t sizemessage = sizeof(struct Message) - sizeof(long);
    printf("Size message : %ld\n", sizemessage);
    if(msgsnd(msgid, &message, sizemessage, 0) == -1)
        errExit("Error sending message");
    printf("Messaggio inviato: %s\nFine...\n", message.mtext);
    return 0;
}

