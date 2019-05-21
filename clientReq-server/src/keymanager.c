#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h> //for file descriptor, servira?
#include "../inc/keymanager.h"

int keymanager() {
    while(1) {
        int tempo = 30;
        while (tempo!=0){
            tempo = sleep(tempo);
        }
        //TODO: KEY MANAGER DELETE TASk!
    }
    return 0;
}

long insertKey(long key, char userCode[]) {
    return 0;
}