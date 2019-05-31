#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../inc/errExit.h"
#include <string.h>


int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma salva!\n"); 
    if(argc == 1)
        errExit("No args");

    
    int fileFD  = open("text", O_CREAT | O_WRONLY | O_TRUNC);
    if(fileFD == -1) {
        errExit("Error open/creation file");
    }
    

    for(int i=1; i<argc; i++) {
        write(fileFD, argv[i], strlen(argv[i]));
        write(fileFD, "\n", strlen("\n"));
    }
    if(close(fileFD) == -1)
        errExit("Error closing file");
    printf("Fine...\n");
    return 0;
}
