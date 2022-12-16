#ifndef TYPES_H
#define TYPES_H

typedef enum
{
    ProcessState_Ready,
    ProcessState_Blocked,
    ProcessState_Running,
    ProcessState_Finished
} ProcessState;

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

    /*** Changes during the process life-time***/
    int remainingTime;  /* Remaining time for the process to finish*/
    int priority;       /* 0 : Highest, 10: Lowest*/
    ProcessState state; /* Current process state*/

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

#endif