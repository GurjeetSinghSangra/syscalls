#ifndef _KEYGENERATOR_HH
#define _KEYGENERATOR_HH

struct Node {
    long value;
    struct Node *next;
};

long generateKey(long requestNumber, int pid, int service);

int containsKey(long key);

int addNode(long key);

long hashcode(long request, int index);

#endif