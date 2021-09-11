#include "segel.h"
#ifndef QUEUE_H
#define QUEUE_H


struct node {
    struct node *next;
    unsigned long arrival;
    void *value;
};
typedef struct node node_t;

void enqueue(void *object);
void* dequeue(unsigned long *dispatch,unsigned long *arrival);

#endif