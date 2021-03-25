/*@Student name: Phi Long Nguyen
 Student ID: 19247171*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "queue.h"
#include <stdbool.h>


/*create a new queue with given capacity*/
Queue* initQueue(int capacity)
{
    Queue* newQueue = (Queue*)calloc(1,sizeof(Queue));
    newQueue->capacity = capacity;
    newQueue->front = 0;
    newQueue->rear = capacity-1;
    newQueue->numItem = 0;
    newQueue->array = (void**)calloc(newQueue->capacity,sizeof(void*));
    return newQueue;
}
/*check if queue is full*/
int isFull(Queue* queue)
{
    return (queue->numItem == queue->capacity);
}
/*check if queue is empty*/
int isEmpty(Queue* queue)
{
    return (queue->numItem == 0);
}
/*add new iteam to queue (enqueue)*/
void enqueue(Queue* queue, void* item)//change to void*
{

    if(isFull(queue))
    {
        printf("the Queue is FULL\n");
    }else{
        queue->rear = (queue->rear +1)%queue->capacity;
        queue->array[queue->rear] = item;//add iteam to array
        queue->numItem = queue->numItem +1;
    }
    
}

/*remove front item and return it*/
void* dequeue(Queue* queue)//change to void* later
{
    if(isEmpty(queue))
    {
        printf("the Queue is EMPTY\n");
        return NULL;//error for now
    }else{
        void* item = queue->array[queue->front];
        queue->front = (queue->front +1)%queue->capacity;
        queue->numItem = queue->numItem -1;
        return item;
    }
}
/*retrieve front item*/
void* front(Queue* queue)
{
    if(isEmpty(queue))
        return NULL;
    return queue->array[queue->front];
}
/*retrieve rear item*/
void* rear(Queue* queue)
{
    if(isEmpty(queue))
        return NULL;
    return queue->array[queue->rear];
}

/*free queue*/
void freeQueue(Queue* queue)
{
    free(queue->array);
    free(queue);
}

