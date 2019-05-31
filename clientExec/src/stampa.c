#include <stdlib.h>
#include <stdio.h>

#include "../inc/errExit.h"

int main (int argc, char *argv[]) {
    printf("Benvenuto sono il programma stampa!\n");
    if(argc == 1)
        errExit("No args");

    for(int i=1; i<argc; i++) {
        printf("Argomento %d: %s\n", i, argv[i]);
    }
    printf("Fine...\n");
    return 0;
}
