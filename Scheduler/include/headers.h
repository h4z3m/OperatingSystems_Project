/**
 * @file headers.h
 * @author yogilany@gmail.com, h4z3m1z@gmail.com, ahmedtarek1754@gmail.com
 * @brief
 * @version 1.0
 * @date 2022-12-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef HEADERS_H
#define HEADERS_H

#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h> //if you don't use scanf/printf change this include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "types.h"

typedef short bool;
#define true 1
#define false 0
node *treeRoot = NULL;
#define MemorySize 1024

#define SHKEY 300
#define MSGQKEY 400
#define SEM_PROC_SCHED_KEY 500

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

#define SCHEDULER_LOG_FILENAME ((const char *)"scheduler.log")
#define SCHEDULER_PERF_FILENAME ((const char *)"scheduler.perf")

#define DEBUG_MODE 1
#if DEBUG_MODE == 1
#define DEBUG_PRINTF(message, ...) printf(message, ##__VA_ARGS__)
#else
#define DEBUG_PRINTF(message, ...)
#endif

///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

/**
 * @brief   Semaphore down operation. Decrements the semaphore value by 1
 *
 * @param sem       Semaphore ID
 */
void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

/**
 * @brief   Semaphore up operation. Adds the value of up_val: -ve/+ve
 *
 * @param sem       Semaphore ID
 * @param up_val    Value to add to semaphore value
 */
void up(int sem, int up_val)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = up_val;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
}

int getClk()
{
    return *shmaddr;
}
/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

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

/**
 * @brief Create a node give it's data
 *
 * @param data the size of the memory the node represents.
 * @param parent a pointer to the parent.
 * @param start the start of the memory the node represents.
 * @return node* the created node
 */
node *newNode(int data, node *parent, int start)
{
    node *node1 = (node *)malloc(sizeof(node));
    node1->data = data;
    node1->left = NULL;
    node1->right = NULL;
    node1->parent = parent;
    node1->startIndx = start;
    node1->endIndx = start + data - 1;
    return (node1);
}

node *allocateRoot(int size, int currentSize, node *grandParent, char dir, int start)
{
    node *parent = newNode(currentSize, grandParent, start);

    if (grandParent != NULL)
    {
        if (dir == 'l')
            grandParent->left = parent;
        else
            grandParent->right = parent;
    }

    node *child = NULL;
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

node *allocateTree(node *root, int size)
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
            node *child = newNode(size, root, root->startIndx);

            root->left = child;

            return child;
        }
        // If the right is free -> Allocate
        else if (root->right == NULL)
        {
            node *child = newNode(size, root, root->startIndx + root->data / 2);

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
            node *left = allocateTree(root->left, size);
            if (left == NULL)
            {
                node *right = allocateTree(root->right, size);
                return right;
            }
            else
                return left;
        }
        // If the tree has left subtree -> If it finds a place, then the desired memory is allocated. Otherwise it call alloc1 for allocation.
        else if (root->left != NULL)
        {
            node *left = allocateTree(root->left, size);
            if (left == NULL)
            {
                node *child = allocateRoot(size, root->data / 2, root, 'r', root->startIndx + root->data / 2);

                return child;
            }
            else
                return left;
        }
        // If the tree has right subtree -> solve(right subtree). If it finds a place,
        //  then the desired memory is allocated. Otherwise it call alloc1 for allocation.
        else
        {
            node *right = allocateTree(root->right, size);
            if (right == NULL)
            {
                node *child = allocateRoot(size, root->data / 2, root, 'l', root->startIndx);

                return child;
            }
            else
                return right;
        }
    }
    else
        return NULL;
}

/**
 * @brief This function used to allocate a piece of memory in the buddy system
 * memory management
 *
 * @param size The desired memory size to allocate
 * @return node* a node pointer to the allocated piece of memory
 */
node *Allocate(int size)
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

int Deallocate_(node *root)
{
    if (root == NULL)
        return 0;

    if (root->left == NULL && root->right == NULL)
    {
        node *parent = root->parent;
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

void Deallocate(node *root)
{
    if (Deallocate_(root) == 0)
        treeRoot = NULL;
}

#endif // HEADERS_H
