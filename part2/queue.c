/*
 * QUEUE CODE
 * -----------------------------------------------------------------------------------------------------------------------------------------------
 */
#include "queue.h"

void queueInit(queue *q, size_t memSize) {
    q->sizeOfQueue = 0;
    q->memSize = memSize;
    q->head = q->tail = NULL;
}

int enqueue(queue *q, const void *data) {
    node *newNode = (node *) malloc(sizeof(node));
    //exit if the malloc failed
    if (newNode == NULL) {
        return -1;
    }

    newNode->data = malloc(q->memSize);
    // when newNode->data malloc failed.
    if (newNode->data == NULL) {
        free(newNode);
        return -1;
    }

    newNode->next = NULL;

    memcpy(newNode->data, data, q->memSize);

    if (q->head == NULL && q->tail == NULL) {
        //Queue was empty
        q->head = q->tail = newNode;
    } else {
        q->tail->next = newNode;
        q->tail = newNode;
    }

    q->sizeOfQueue++;
    return 0;
}

void dequeue(queue *q, void *data) {
    if (q->sizeOfQueue > 0) {
        node *temp = q->head;
        memcpy(data, temp->data, q->memSize);
        if (q->head == q->tail) {
            //only one element in queue
            q->head = NULL;
            q->tail = NULL;
        }
        if (q->sizeOfQueue > 1) {
            q->head = q->head->next;
        }
        q->sizeOfQueue--;
        free(temp->data);
        free(temp);
    }
}

void queuePeek(queue *q, void *data) {
    if (q->sizeOfQueue > 0) {
        node *temp = q->head;
        memcpy(data, temp->data, q->memSize);
    }
}



