/**
 * @file data_structures.c
 * @author yogilany@gmail.com, h4z3m1z@gmail.com, ahmedtarek1754@gmail.com
 * @brief
 * @version 1.0
 * @date 2022-12-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../include/data_structures.h"
#include "../include/types.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/************************************************* Queue *************************************************/

Queue *createQueue()
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));

    queue->capacity = 0;
    queue->front = queue->rear = NULL;
    return queue;
}

bool isEmpty(Queue *queue)
{
    return (queue->front == NULL);
}

void enqueue(Queue *q, void *dataPtr)
{
    Node *link = (Node *)malloc(sizeof(Node));
    link->dataPtr = dataPtr;
    link->nextNode = NULL;
    // Empty queue,
    q->capacity++;
    if (q->front == NULL)
    {
        q->front = link;
        q->rear = link;
        return;
    }
    // Filled queue

    // Make the previous rear point to the newly created node
    q->rear->nextNode = link;
    // Change rear to be the new link
    q->rear = link;
}

bool dequeue(Queue *queue, void **out_data)
{
    if (queue->front == NULL)
    {
        return 0;
    }
    Node *deqNode = queue->front;
    queue->front = queue->front->nextNode;

    *out_data = deqNode->dataPtr;

    free(deqNode);
    queue->capacity--;

    return 1;
}

bool peek(Queue *q, void **dataPtr)
{
    if (isEmpty(q))
        return 0;
    *dataPtr = q->front->dataPtr;
    return 1;
}

void destroyQueue(Queue *q)
{
    void *temp = NULL;
    while (q->front != NULL)
    {
        dequeue(q, &temp);
        if (temp)
            free(temp);
    }
}

/************************************************* Priority Queue *************************************************/

PriorityQueue *createPriorityQueue()
{
    PriorityQueue *pqueue = (PriorityQueue *)malloc(sizeof(PriorityQueue));

    pqueue->capacity = 0;
    pqueue->front = pqueue->rear = NULL;
    return pqueue;
}

bool priorityIsEmpty(PriorityQueue *q)
{
    return (q->front == NULL);
}

void enqueuePriority(PriorityQueue *q, void *dataPtr, int priority)
{
    PrioNode *start = q->front;

    // Create new Node
    PrioNode *temp = (PrioNode *)malloc(sizeof(PrioNode));
    temp->priority = priority;
    temp->dataPtr = dataPtr;
    q->capacity++;
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
    if (q->front->priority > priority)
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
               start->nextNode->priority < priority)
        {
            start = start->nextNode;
        }

        // Either at the ends of the list
        // or at required position
        temp->nextNode = start->nextNode;
        start->nextNode = temp;
    }
}

bool peekPriority(PriorityQueue *q, void **out_data)
{
    if (priorityIsEmpty(q))
        return 0;

    *out_data = q->front->dataPtr;
    return 1;
}

bool dequeuePriority(PriorityQueue *q, void **out_data)
{

    if (priorityIsEmpty(q))
        return 0;

    PrioNode *nodeToDeletePtr = q->front;
    *out_data = q->front->dataPtr;
    q->front = q->front->nextNode;
    // Queue is not empty; remove front
    if (nodeToDeletePtr == q->rear) // Special case: last node in the queue
        q->rear = NULL;
    // Free memory reserved for the dequeued node
    free(nodeToDeletePtr);
    q->capacity--;
    return 1;
}

void removeNodePriority(PriorityQueue *q, void **dataToDelete)
{
    PrioNode *deletedNode = q->front;
    if (q->front == NULL)
    {
        return;
    }

    // Check if the first node is the one to be deleted
    if (q->front->dataPtr == *dataToDelete)
    {
        deletedNode = q->front;
        q->front = q->front->nextNode;
        free(deletedNode);
        return;
    }
    PrioNode *previous = q->front;
    deletedNode = q->front->nextNode;
    while (deletedNode != NULL)
    {
        if (*dataToDelete == deletedNode->dataPtr)
        {

            previous->nextNode = deletedNode->nextNode;
            free(deletedNode);
            return;
        }
        previous = deletedNode;
        deletedNode = deletedNode->nextNode;
    }
}

void destroyPriorityQueue(PriorityQueue *q)
{
    void *temp;

    // Free (Dequeue) all nodes in the queue
    while (dequeuePriority(q, &temp))
    {
        if (temp)
            free(temp);
    }
}

/************************************************* Circular Queue *************************************************/

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
    q->capacity++;
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

    *out_data = q->rear->dataPtr;
    return 1;
}

bool dequeueCircularFront(CircularQueue *q, void **out_data)
{
    if (q->front == NULL)
    {
        return 0;
    }

    // If this is the last node to be deleted
    if (q->front == q->rear)
    {
        *out_data = q->front->dataPtr;
        free(q->front);
        q->front = NULL;
        q->rear = NULL;
    }
    else // There are more than one nodes
    {
        Node *temp = q->front;
        *out_data = temp->dataPtr;
        q->front = q->front->nextNode;
        q->rear->nextNode = q->front;
        free(temp);
    }
    q->capacity--;

    return 1;
}

bool dequeueCircular(CircularQueue *q, void *deleted_data)
{
    if (q->front == NULL)
    {
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
    q->capacity--;
    return 1;
}

void destroyCircularQueue(CircularQueue *q)
{
    void *temp;

    // Free (Dequeue) all nodes in the queue
    while (dequeueCircularFront(q, &temp))
    {
        if (temp)
            free(temp);
    }
}

/************************************************* Multilevel Queue *************************************************/
MultiLevelQueue *createMultiLevelQueue()
{
    // all 11 levels (priority from 0 to 10) levels are pointing to null
    MultiLevelQueue *mlfl = (MultiLevelQueue *)malloc(sizeof(MultiLevelQueue));
    for (int i = 0; i < 11; i++)
    {
        Queue *newQueuePtr = createQueue();
        mlfl->Qptrs[i] = newQueuePtr;
    }
    return mlfl;
}

// this function returns the first occupied level in the queue , otherwise ; it returns -1
int multiLevelisEmpty(MultiLevelQueue *q)
{
    int first_occupied_level = -1;
    for (int i = 0; i < 11; i++)
    {
        if(q->Qptrs[i]->front!=NULL)
            return first_occupied_level = i;
    }
    return first_occupied_level;
}
// uses queue DS to enqueue in the correct level
void enqueueMultiLevel(MultiLevelQueue *q, void **dataPtr, int priority_level)
{
    return enqueue(q->Qptrs[priority_level], *dataPtr);
}

// dequeue in case of process done
bool dequeueMultiLevel(MultiLevelQueue *q, void **out_data, int priority_level)
{
    return dequeue(q->Qptrs[priority_level], out_data);
}

void destroyMultiLevelQueue(MultiLevelQueue *q)
{
    for (int i = 0; i < 11; i++)
    {
        destroyQueue(q->Qptrs[i]);
    }
}