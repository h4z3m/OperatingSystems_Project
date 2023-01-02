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
    if (q->front->priority >= priority)
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
               start->nextNode->priority <= priority)
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
        if (q->Qptrs[i]->front != NULL)
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

/************************************************* Buddy system tree *************************************************/


treeNode *treeRoot = (void *)(0);

unsigned int nextPowerOf2(unsigned int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

treeNode *newNode(int data, treeNode *parent, int start)
{
    treeNode *node1 = (treeNode *)malloc(sizeof(treeNode));
    node1->data = data;
    node1->left = NULL;
    node1->right = NULL;
    node1->parent = parent;
    node1->startIndx = start;
    node1->endIndx = start + data - 1;
    return (node1);
}

treeNode *allocateRoot(int size, int currentSize, treeNode *grandParent, char dir, int start)
{
    treeNode *parent = newNode(currentSize, grandParent, start);

    if (grandParent != NULL)
    {
        if (dir == 'l')
            grandParent->left = parent;
        else
            grandParent->right = parent;
    }

    treeNode *child = NULL;
    while (currentSize > size)
    {
        child = newNode(currentSize / 2, parent, start);

        parent->left = child;
        parent = child;

        currentSize = currentSize / 2;
    }

    if (child == NULL)
        return parent;
    return child;
}

treeNode *allocateTree(treeNode *root, int size)
{
    if (root == NULL)
        return NULL;

    // allocated node
    if (root->left == NULL && root->right == NULL)
        return NULL;

    // if the this node has a size that equals to the double of the desired size
    if (root->data == 2 * size)
    {
        // If the left is free -> Allocate
        if (root->left == NULL)
        {
            treeNode *child = newNode(size, root, root->startIndx);

            root->left = child;

            return child;
        }
        // If the right is free -> Allocate
        else if (root->right == NULL)
        {
            treeNode *child = newNode(size, root, root->startIndx + root->data / 2);

            root->right = child;

            return child;
        }
        else
            return NULL;
    }
    // if the this node has a size that is bigger than the double of the desired size
    else if (root->data > 2 * size)
    {

        if (root->left != NULL && root->right != NULL)
        {
            treeNode *left = allocateTree(root->left, size);
            if (left == NULL)
            {
                treeNode *right = allocateTree(root->right, size);
                return right;
            }
            else
                return left;
        }
        // If the tree has left subtree -> If it finds a place, then the desired memory is allocated. Otherwise it call alloc1 for allocation.
        else if (root->left != NULL)
        {
            treeNode *left = allocateTree(root->left, size);
            if (left == NULL)
            {
                treeNode *child = allocateRoot(size, root->data / 2, root, 'r', root->startIndx + root->data / 2);

                return child;
            }
            else
                return left;
        }
        // If the tree has right subtree -> solve(right subtree). If it finds a place,
        //  then the desired memory is allocated. Otherwise it call alloc1 for allocation.
        else
        {
            treeNode *right = allocateTree(root->right, size);
            if (right == NULL)
            {
                treeNode *child = allocateRoot(size, root->data / 2, root, 'l', root->startIndx);

                return child;
            }
            else
                return right;
        }
    }
    else
        return NULL;
}

treeNode *Allocate(int size)
{
    int blockSize = nextPowerOf2(size);

    if (treeRoot == NULL)
    {
        treeRoot = newNode(MemorySize, NULL, 0);
        if (MemorySize > blockSize)
            return allocateRoot(blockSize, MemorySize / 2, treeRoot, 'l', 0);
        else
            return treeRoot;
    }
    else
        return allocateTree(treeRoot, blockSize);
}

int Deallocate_(treeNode *root)
{
    if (root == NULL)
        return 0;

    if (root->left == NULL && root->right == NULL)
    {
        treeNode *parent = root->parent;
        if (parent != NULL)
        {
            if (parent->right == root)
                parent->right = NULL;
            else
                parent->left = NULL;
        }
        free(root);
        return Deallocate_(parent);
    }

    return 1;
}

void Deallocate(treeNode *root)
{
    if (Deallocate_(root) == 0)
        treeRoot = NULL;
}
