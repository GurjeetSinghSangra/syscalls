#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>

#include "../inc/errExit.h"

int main (int argc, char *argv[]) {
    if(argc != 4) {
        errExit("Wrong arugments number");
    }

    char *userCode = argv[1];
    long key = strtol(argv[2], NULL, 10);
    char *serviceInput = argv[3];

    //----MEMORIA CONDIVISA----
    //ACCEDI ALLA MEMORIA CONDIVISA, Attach, utilizzo dei semafori
    //CERCA CHIAVE 
    //Se non trovata -> controlla se non esiste o se e` gia stata usata (Login delete)
    //Se trovata CANCELLA la chiave dalla memoria condivisa
    //DETACH
    //NEL caso in cui la chiava e` stata trovata fai l'exec del singolo servizio!
    
    
    //IF key found
    if(strcmp(serviceInput, "Stampa") == 0) {
        execl("stampa", "stampa", userCode, argv[2], (char *)NULL);
    } else if(strcmp(serviceInput, "Invia") == 0) {
        execl("invia", "invia", userCode, argv[2], (char *)NULL);
    } else if(strcmp(serviceInput, "Salva") == 0) {
        execl("salva", "salva", userCode, argv[2], (char *)NULL);
    } else {
        printf("Wrong service received");
    }
    errExit("Exec failed");
    return 0;
}
