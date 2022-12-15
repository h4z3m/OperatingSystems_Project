#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H
typedef short bool;
typedef struct 
{
    void *dataPtr;
    struct Node *nextNode;
}Node;

typedef struct
{
    void *dataPtr;
    int priority;
    struct Prio_Node *nextNode;
} Prio_Node;

typedef struct
{
    Node* front, rear;
    unsigned capacity;
    Node *nextNode

} Queue;

typedef struct
{
    Prio_Node *front;
    Prio_Node *back;

} priorityQueue;

typedef struct
{
    int levels;
    Queue *queues[];
} multiLevelQueue;

/* Queue */
queue *createQueue(){
    Node *p =(Node*) malloc(sizeof(Node));
    p->dataPtr=(void*)(0);
    
}
bool isEmpty(queue *q);
void enqueue(queue *q, void *dataPtr);
Node *peek(queue *q);
Node *dequeue(queue *q);
void destroyQueue(queue *q);
Queue *createQueue(unsigned capacity);
int isEmpty(Queue *queue);
void enqueue(Queue *queue, int item);
int dequeue(Queue *queue);
void destroyQueue(Queue *q);
int front(Queue *queue);
int rear(Queue *queue);

/* Priority queue */
priorityQueue *createPriorityQueue();
bool priorityIsEmpty(priorityQueue *q);
void enqueuePriority(priorityQueue *q, void *dataPtr, int priority);
Node *peekPriority(priorityQueue *q);
Node *dequeuePriority(priorityQueue *q);
void destroyPriorityQueue(priorityQueue *q);

/* Multi level queue*/
multiLevelQueue *createMultiLevelQueue(int size);
bool multiLevelisEmpty(multiLevelQueue *q);
void enqueueMultiLevel(multiLevelQueue *q, void *dataPtr);
Node *peekMultiLevel(multiLevelQueue *q, int level);
Node *dequeueMultiLevel(multiLevelQueue *q, int level);
void destroyMultiLevel(multiLevelQueue *q);

#endif // DATA_STRUCTURES_H