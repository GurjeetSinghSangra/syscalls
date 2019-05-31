#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../inc/keygenerator.h"

struct Node *head = NULL;

long generateKey(long requestNumber, short service) {
    long key = requestNumber;
    //generate hash
    if(head == NULL) {
        head = malloc(sizeof(struct Node));
        head->value = key;
    } else {
        if(containsKey(key) == 1) {
            //generate second hash or increment of 1 until  a not found
            do {

            } while(containsKey(key) == 1);
        }
        //add key 
        addNode(key);
    }
    return key;
}

long hashcode(long requestNumber, int service) {
    return 1;
}

int containsKey(long key) {
    struct Node *node = head;//DO i copy or get the reference?
    int pos = 1;
    while(node != NULL) {
        printf("looking at pos %d\n", pos);
        if(node->value == key) {
            printf("Found same key\n");
            return 1;
        } else {
            printf("Next \n");
            node = node->next;
        }
        pos++;
    }
    return 0;
}

int addNode(long key) {
    struct Node *node = head;
    while(node != NULL) {
        node = node->next;
    }
    node->next = malloc(sizeof(struct Node));
    node->next->value = key;
    node->next->next = NULL;

    return 1;
}