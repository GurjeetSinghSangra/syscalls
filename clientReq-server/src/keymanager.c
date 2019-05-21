#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h> //for file descriptor, servira?


int keymanager() {
    while(1) {
        int tempo = 30;
        while (tempo!=0){
            tempo = sleep(tempo);
        }
        printf("30 secoonds has been passed! Key manager Task begin\n");
        //TODO: KEY MANAGER DELETE TASk!
    }
    return 0;
}

int insertKey(int key, char userCode[]) {
    return 0;
}