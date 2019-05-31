#ifndef _KEYGENERATOR_HH
#define _KEYGENERATOR_HH

struct Node {
    long value;
    struct Node *next;
};

long generateKey(long requestNumber, short service);

int containsKey(long key);

int addNode(long key);

long hashcode(long requestNumber, int service);

#endif