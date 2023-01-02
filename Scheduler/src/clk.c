/*
 * This file is done for you.
 * Probably you will not need to change anything.
 * This file represents an emulated clock for simulation purpose only.
 * It is not a real part of the operating system!
 */

#include "../include/headers.h"
// #include "time.h"
int shmid;

/* Clear the resources before exit */
void cleanup(int signum)
{
    shmctl(shmid, IPC_RMID, NULL);
    printf("Clock Terminating!...\n");
    exit(0);
}

/* This file represents the system clock for ease of calculations */
int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    initClkSem(&clk_sem_id);
    union Semun semun = {0};
    semun.val = 0;
    if (semctl(clk_sem_id, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl\n");
        exit(-1);
    }
    // initClkUsers();
    printf("Clock Starting...\n");
    signal(SIGINT, cleanup);
    int clk = 0;
    // Create shared memory for one integer variable 4 bytes
    shmid = shmget(SHKEY, 4, IPC_CREAT | 0644);
    if ((long)shmid == -1)
    {
        perror("Error in creating shm!");
        exit(-1);
    }
    int *shmaddr = (int *)shmat(shmid, (void *)0, 0);
    if ((long)shmaddr == -1)
    {
        perror("Error in attaching the shm in clock!");
        exit(-1);
    }
    *shmaddr = clk; /* Initialize shared memory */

    // int old_users = getClkUsers();
    sleep(5);

    while (1)
    {
        usleep(1000 * 100);
        // sleep(1);
        // DEBUG_PRINTF("[CLK] Current clk users = %d\n", getClkUsers());
        // old_users = getClkUsers();
        // down_value(clk_sem_id, -old_users);
        // /* If new processes arrive while we were trying to down, wait for them*/
        // if (old_users < getClkUsers())
        // {
        //     down_value(clk_sem_id, -(getClkUsers() - old_users));
        // }
        // sleep(1);
        (*shmaddr)++;
    }
}
