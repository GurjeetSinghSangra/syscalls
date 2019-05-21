
#include "constant.h"
#include <stdio.h>
#include <sys/types.h>


struct Request {
    char user_code[USER_CODE_LENGTH];
    char service[USER_CODE_LENGTH];
    pid_t pid;
};

struct Response {
    int key;
    int noservice; //FLAG field, if it has value 1, the requested service is NOT avvailable.
};