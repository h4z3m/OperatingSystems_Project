#include "../include/data_structures.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

Queue *createQueue()
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));

    queue->capacity = 0;
    queue->front = queue->rear = NULL;
    return queue;
}

// Queue is empty when size is 0
bool isEmpty(Queue *queue)
{
    return (queue->capacity == 0);
}

// Function to add an item to the queue.
// It changes rear and size
void enqueue(Queue *q, void *dataPtr)
{
    Node *link = malloc(sizeof(Node));
    link->dataPtr = dataPtr;
    link->nextNode = NULL;
    // Empty queue,
    if (q->front == NULL)
    {
        q->front = link;
        q->rear = link;
        printf("Queue is empty, queueing first element...\n");
        return;
    }
    // Filled queue

    // Make the previous rear point to the newly created node
    q->rear->nextNode = link;
    // Change rear to be the new link
    q->rear = link;
}

// Function to remove an item from queue.
// It changes front and size
bool dequeue(Queue *queue, void **out_data)
{
    if (queue->front == NULL)
    {
        printf("Queue is empty.");
        return 0;
    }
    Node *deqNode = queue->front;
    queue->front = queue->front->nextNode;

    *out_data = deqNode->dataPtr;

    free(deqNode);
    return 1;
}

bool peek(Queue *q, void *dataPtr)
{
    if (isEmpty(q))
        return 0;
    if (!dataPtr)
        dataPtr = q->front->dataPtr;
    return 1;
}

void destroyQueue(Queue *q)
{
    while (q->front != NULL)
    {
        dequeue(q, NULL);
    }
}

/* Priority Queue */
PriorityQueue *createPriorityQueue()
{
    PriorityQueue *pqueue = (PriorityQueue *)malloc(sizeof(PriorityQueue));

    pqueue->capacity = 0;
    pqueue->front = pqueue->rear = NULL;
    return pqueue;
}

bool priorityIsEmpty(PriorityQueue *q)
{
    return (q->capacity == 0);
}

void enqueuePriority(PriorityQueue *q, void *dataPtr, int priority)
{
    PrioNode *start = q->front;

    // Create new Node
    PrioNode *temp = (PrioNode *)malloc(sizeof(PrioNode));
    if (!q->front)
    {
        q->front = temp;
        q->front->priority = priority;
        return;
    }
    // Special Case: The head of list has
    // lesser priority than new node. So
    // insert newnode before head node
    // and change head node.
    if (q->front->priority < priority)
    {

        // Insert New Node before head
        temp->nextNode = q->front;
        q->front = temp;
    }
    else
    {
        // Traverse the list and find a
        // position to insert new node
        while (start->nextNode != NULL &&
               start->nextNode->priority > priority)
        {
            start = start->nextNode;
        }

        // Either at the ends of the list
        // or at required position
        temp->nextNode = start->nextNode;
        start->nextNode = temp;
    }
}

bool peekPriority(PriorityQueue *q, void *out_data)
{
    if (priorityIsEmpty(q))
        return 0;

    out_data = q->front->dataPtr;
    return 1;
}

bool dequeuePriority(PriorityQueue *q, void **out_data)
{

    if (priorityIsEmpty(q))
        return 0;

    PrioNode *nodeToDeletePtr = q->front;
    if (!out_data)
        *out_data = q->front->dataPtr;
    q->front = q->front->nextNode;
    // Queue is not empty; remove front
    if (nodeToDeletePtr == q->rear) // Special case: last node in the queue
        q->rear = NULL;
    // Free memory reserved for the dequeued node
    free(nodeToDeletePtr);
    return 1;
}

void destroyPriorityQueue(PriorityQueue *q)
{
    void *temp;

    // Free (Dequeue) all nodes in the queue
    while (dequeuePriority(q, temp))
    {
        if (temp)
            free(temp);
    }
}

/* Circular Queue*/
CircularQueue *createCircularQueue()
{

    CircularQueue *cQueue = (CircularQueue *)malloc(sizeof(CircularQueue));
    cQueue->front = cQueue->rear = NULL;
    cQueue->capacity = 0;
    return cQueue;
}

bool circularIsEmpty(CircularQueue *q)
{
    return (q->front == NULL);
}

void enqueueCircular(CircularQueue *q, void *dataPtr)
{
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->dataPtr = dataPtr;

    if (q->front == NULL)
        q->front = temp;
    else
        q->rear->nextNode = temp;

    q->rear = temp;
    q->rear->nextNode = q->front;
}

bool peekFront(CircularQueue *q, void **out_data)
{
    if (circularIsEmpty(q))
        return 0;

    *out_data = q->front->dataPtr;
    return 1;
}

bool peekRear(CircularQueue *q, void **out_data)
{
    if (circularIsEmpty(q))
        return 0;

    if (!out_data)
        *out_data = q->front->dataPtr;
    return 1;
}

bool peekCircular(CircularQueue *q, void *out_data, unsigned int offset)
{
    if (offset == 0)
        return 0;

    static Node *start = NULL;
}

bool dequeueCircularFront(CircularQueue *q, void **out_data)
{
    if (q->front == NULL)
    {
        printf("Queue is empty");
        return 0;
    }

    // If this is the last node to be deleted
    if (q->front == q->rear)
    {
        if (!out_data)
            *out_data = q->front->dataPtr;
        free(q->front);
        q->front = NULL;
        q->rear = NULL;
    }
    else // There are more than one nodes
    {
        Node *temp = q->front;
        out_data = temp->dataPtr;
        q->front = q->front->nextNode;
        q->rear->nextNode = q->front;
        free(temp);
    }

    return 1;
}
bool dequeueCircular(CircularQueue *q, void *deleted_data)
{
    if (q->front == NULL)
    {
        printf("Queue is empty");
        return 0;
    }

    Node *curr = q->front, *prev;
    while (curr->dataPtr != deleted_data)
    {
        if (curr->nextNode == q->front)
        {
            break;
        }

        prev = curr;
        curr = curr->nextNode;
    }

    // Check if node is only node
    if (curr->nextNode == q->front)
    {
        q->front = NULL;
        free(curr);
        return 1;
    }

    // If more than one node, check if
    // it is first node
    if (curr == q->front)
    {
        prev = q->front;
        while (prev->nextNode != q->front)
            prev = prev->nextNode;
        q->front = curr->nextNode;
        prev->nextNode = q->front;
        free(curr);
    }

    // check if node is last node
    else if (curr->nextNode == q->front && curr == q->front)
    {
        prev->nextNode = q->front;
        free(curr);
    }
    else
    {
        prev->nextNode = curr->nextNode;
        free(curr);
    }

    return 1;
}
void destroyCircularQueue(CircularQueue *q);