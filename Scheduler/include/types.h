/**
 * @file types.h
 * @author yogilany@gmail.com, h4z3m1z@gmail.com, ahmedtarek1754@gmail.com
 * @brief
 * @version 1.0
 * @date 2022-12-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef TYPES_H
#define TYPES_H

typedef enum
{
    ProcessState_Ready,
    ProcessState_Blocked,
    ProcessState_Running,
    ProcessState_Finished
} ProcessState;

typedef struct node
{
    int data;            /**< the size of the node */
    int startIndx;       /**< the start of the memory piece */
    int endIndx;         /**< the end of the memory piece */
    struct node *left;   /**< a node representing the left subtree */
    struct node *parent; /**< a node to the parent; equal NULL if there's no parent */
    struct node *right;  /**< a node representing the right subtree */
} node;

typedef struct
{
    /*** Unchanged during execution ***/
    int PID;           /* Unique ID of the process*/
    int inputPID;      /* Input ID of the process from text file*/
    int arrivalTime;   /* Arrival time of the process*/
    int executionTime; /* Total runtime of the process*/
    int memSize;       /* Total allocated space for the process (for phase 2)*/
    int finishTime;    /* Finish time of the process*/

    /*** Changed only when the process starts for the first time ***/
    int startTime; /* Time at which the process STARTS execution*/
    int sem_id;
    /*** Changes during the process life-time***/
    int remainingTime;  /* Remaining time for the process to finish*/
    int priority;       /* 0 : Highest, 10: Lowest*/
    ProcessState state; /* Current process state*/
    node *memoryNode;

} ProcessControlBlock;

struct msgbuf
{
    long mtype;
    ProcessControlBlock pcb;
};

typedef enum
{
    SchedulingAlgorithm_SJF = 1, /* Shortest Job First*/
    SchedulingAlgorithm_HPF,     /* Highest Priority First*/
    SchedulingAlgorithm_RR,      /* Round Robin*/
    SchedulingAlgorithm_MLFL     /* Multi-Level Feedback Loop*/

} SchedulingAlgorithm_t;

/* arg for semctl system calls. */
union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    unsigned short *array; /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

#endif
