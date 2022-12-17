#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

typedef short bool;
typedef struct _node
{
    void *dataPtr;
    struct _node *nextNode;
} Node;

typedef struct _prio_node
{
    void *dataPtr;
    int priority;
    struct _prio_node *nextNode;
} PrioNode;

typedef struct
{
    Node *front;
    Node *rear;
    unsigned capacity;

} Queue;

typedef struct
{
    PrioNode *front;
    PrioNode *rear;
    unsigned capacity;
} PriorityQueue;

typedef struct
{
    Node *front;
    Node *rear;
    unsigned capacity;

} CircularQueue;

typedef struct
{
    int levels;
    Queue queues[10];
} MultiLevelQueue;

/* Queue */
Queue *createQueue();

bool isEmpty(Queue *q);
void enqueue(Queue *q, void *dataPtr);
bool peek(Queue *q, void **dataPtr);
bool dequeue(Queue *queue, void **out_data);
void destroyQueue(Queue *q);

/* Priority Queue */
PriorityQueue *createPriorityQueue();

bool priorityIsEmpty(PriorityQueue *q);
void enqueuePriority(PriorityQueue *q, void *dataPtr, int priority);
bool peekPriority(PriorityQueue *q, void **out_data);
bool dequeuePriority(PriorityQueue *q, void **out_data);
void destroyPriorityQueue(PriorityQueue *q);

/* Circular Queue*/
CircularQueue *createCircularQueue();

bool circularIsEmpty(CircularQueue *q);
void enqueueCircular(CircularQueue *q, void *dataPtr);
bool peekFront(CircularQueue *q, void **out_data);
bool peekRear(CircularQueue *q, void **out_data);
bool dequeueCircularFront(CircularQueue *q, void **out_data);
bool dequeueCircular(CircularQueue *q, void *deleted_data);
void destroyCircularQueue(CircularQueue *q);

/* Multi level Queue*/
MultiLevelQueue *createMultiLevelQueue(int levels);

bool multiLevelisEmpty(MultiLevelQueue *q);
void enqueueMultiLevel(MultiLevelQueue *q, void **dataPtr);
bool peekMultiLevel(MultiLevelQueue *q, int level, void **out_data);
bool dequeueMultiLevel(MultiLevelQueue *q, int level, void **out_data);
void destroyMultiLevelQueue(MultiLevelQueue *q);

#endif // DATA_STRUCTURES_H