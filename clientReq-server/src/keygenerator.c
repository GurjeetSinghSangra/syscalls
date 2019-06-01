#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../inc/keygenerator.h"

struct Node *head = NULL;

long generateKey(long requestNumber, short service) {
    long key = requestNumber;
    //generate hash
    if(containsKey(key) == 1) {
        printf("The generation of hash has created a replica.\n We have to generate a new one. \n");

        do {

        } while(containsKey(key) == 1);
    }
    //add key 
    addNode(key);
    
    return key;
}

long hashcode(long requestNumber, int service) {
    return 1;
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

