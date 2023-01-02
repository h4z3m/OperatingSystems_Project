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

#include "types.h"
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

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define MSGQKEY 400
#define SEM_PROC_SCHED_KEY 500
#define SEM_CLK_KEY 600
#define SHM_USERS_COUNT 700

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
int *shmaddr;        //
int *shm_users_addr; //
//===============================

/* Clock semaphore ID */
int clk_sem_id = 0;

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
 * @brief   Semaphore down operation. Decrements the semaphore value by 1
 *
 * @param sem       Semaphore ID
 */
void down_value(int sem, int val)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = val;
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

int getClkUsers()
{
    return (*shm_users_addr);
}

void enterClkUsers()
{
    (*shm_users_addr)++;
}

void exitClkUsers()
{
    (*shm_users_addr)--;
}

void destroySem(int semid)
{
    union Semun semun;
    if (semctl(semid, 0, IPC_RMID, semun) == -1)
    {
        perror("[SCHEDULER] Error in semctl - deInitProcSem()\n");
        exit(-1);
    }
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

int initClkUsers()
{
    int shmid = shmget(SHM_USERS_COUNT, 4, IPC_CREAT | 0666);
    if ((long)shmid == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    shm_users_addr = (int *)shmat(shmid, (void *)0, 0);
    if ((long)shm_users_addr == -1)
    {
        perror("Error in attaching the shm in clock!");
        exit(-1);
    }
    return shmid;
}

void initClkSem(int *semid)
{
    int key_id = ftok("./", getpid());

    if (key_id == -1)
        perror("Error in ftok\n");

    *semid = semget(SEM_CLK_KEY, 1, 0666 | IPC_CREAT);

    if (*semid == -1)
    {
        perror("Error in create sem\n");
        exit(-1);
    }
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

#endif // HEADERS_H
