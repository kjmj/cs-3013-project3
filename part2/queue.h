#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    void *data;
    struct node *next;
};
typedef struct node node;

struct queue {
    int sizeOfQueue;
    size_t memSize;
    node *head;
    node *tail;
};
typedef struct queue queue;

/*
 * QUEUE CODE
 * -----------------------------------------------------------------------------------------------------------------------------------------------
 */
void queueInit(queue *q, size_t memSize);

int enqueue(queue *, const void *);

void dequeue(queue *, void *);

void queuePeek(queue *, void *);
