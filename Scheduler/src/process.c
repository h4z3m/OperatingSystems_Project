#include "../include/headers.h"
#include "../include/types.h"

/* Modify this file as needed*/
int remainingtime;

int sem_id;

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
    int pid = atoi(argv[1]);
    remainingtime = atoi(argv[2]);
    DEBUG_PRINTF(RED "[PROCESS %d] Created, remaining time =%d\n" RESET, pid, remainingtime);
    initClk();
    getSem(&sem_id);

    // remainingtime = ??;
    int currClk = -1;
    /* Initially set the semaphore to zero to simulate blocked process*/
    union Semun semun = {0};
    semun.val = 0;
    if (semctl(sem_id, 0, SETVAL, semun) == -1)
    {
        perror("[SCHEDULER] Error in semctl\n");
        exit(-1);
    }
    /* Up semaphore to synchronize with scheduler */
    up(sem_id, 1);

    while (remainingtime > 0)
    {
        /* Process tries to decrement semaphore, blocks until becomes non zero with allowed run time*/
        down(sem_id);
        DEBUG_PRINTF(RED "[PROCESS %d] Downed, remaining time =%d\n" RESET, pid, remainingtime);
        remainingtime--;
        currClk = getClk();
        while (currClk == getClk())
            ;
    }

    destroyClk(false);
    DEBUG_PRINTF(RED "[PROCESS %d] Terminating...\n" RESET, pid);

    return 0;
}
