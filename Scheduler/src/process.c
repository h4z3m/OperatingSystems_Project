/**
 * @file process.c
 * @author yogilany@gmail.com, h4z3m1z@gmail.com, ahmedtarek1754@gmail.com
 * @brief
 * @version 1.0
 * @date 2022-12-27
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "../include/headers.h"
#include "../include/types.h"

/* Modify this file as needed*/
int remainingtime;

int sem_id;
int pid;

void getSem(int *semid)
{
    int key_id = ftok("./", getpid());

    if (key_id == -1)
        perror("[PROCESS] Error in ftok\n");

    *semid = semget(key_id, 1, 0666 | IPC_CREAT);

    if (*semid == -1)
    {
        perror("[PROCESS] Error in create sem\n");
        exit(-1);
    }
}

void SIGCONT_HANDLER(int signum)
{

    DEBUG_PRINTF(GRN "[PROCESS %d] Received continue signal\n" RESET, pid);
    signal(SIGCONT, SIGCONT_HANDLER);
}
/**
 * @brief This file simulates the process which needs to be CPU bound.
 * It runs for some time depending on the algorithm then gives back control to the
 * scheduler to switch to a new process.
 *
 * @param agrc Argument count
 * @param argv Argument vector: Contains total time for this process, decrements every clk cycle
 * @return int
 */
int main(int agrc, char *argv[])
{

    /* Setup handlers*/
    // signal(SIGCONT, SIGCONT_HANDLER);

    pid = atoi(argv[1]);
    remainingtime = atoi(argv[2]);
    DEBUG_PRINTF(RED "[PROCESS %d] Created, remaining time = %d\n" RESET, pid, remainingtime);
    initClk();

    int currClk = -1;
    while (remainingtime > 0)
    {
        currClk = getClk();
        DEBUG_PRINTF(RED "[PROCESS %d] Remaining time = %d\n" RESET, pid, remainingtime);
        remainingtime--;
        while (currClk == getClk())
            ;
    }

    destroyClk(false);
    DEBUG_PRINTF(RED "[PROCESS %d] Terminating...\n" RESET, pid);

    exit(0);
}
