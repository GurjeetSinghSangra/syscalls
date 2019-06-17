#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../inc/keygenerator.h"

#define PRIME 701
#define SECOND_PRIME 700

struct Node *head = NULL;

//KEY lowest decimal represents the service used
long generateKey(long requestNumber, int pid, int service) {
    //generate hash
    int index = 0;
    int inputHascode = requestNumber + pid;
    if(inputHascode == PRIME)//the generated key has not to be 0
        inputHascode++;
    long key = hashcode(inputHascode, index);
    key = key *10 + service;//offset for service
    
    while(containsKey(key) == 1) {
        printf("The generation of hash has created a replica: %li.\n", key);
        index++;
        key = hashcode(inputHascode, index);
        key = key *10 + service;//offset for service
        printf("We have to generate a new one: %li.\n", key);
    }
    addNode(key);
    return key;
}

//hash code  h(k, i) = h1(k) + i * h2(k) mod m'
long hashcode(long request, int index) {
    long code = request % PRIME; //first hash
    code = code + index*( 1 + request % SECOND_PRIME); //second hash
    return code;
}

int containsKey(long key) {
    struct Node *node = head;
    while(node != NULL) {
        if(node->value == key) {
            printf("Found same key\n");
            return 1;
        } else {
            node = node->next;
        }
    }
    return 0;
}

int addNode(long key) {
    struct Node *new_node = malloc(sizeof(struct Node*));
    new_node->value = key;
    new_node->next = NULL;

    if(head == NULL) {
        head = new_node;
        return 1;
    }
    struct Node *last = head;
    while(last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
    return 1;
}

