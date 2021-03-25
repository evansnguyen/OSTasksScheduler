/*@Student name: Phi Long Nguyen
 Student ID: 19247171*/
typedef struct{
    int front, rear, numItem;
    int capacity;
    void** array;//generic
}Queue;
Queue* initQueue(int capacity);
int isFull(Queue* queue);
int isEmpty(Queue* queue);
void enqueue(Queue* queue, void* item);
void* dequeue(Queue* queue);
void* front(Queue* queue);
void* rear(Queue* queue);
void freeQueue(Queue* queue);