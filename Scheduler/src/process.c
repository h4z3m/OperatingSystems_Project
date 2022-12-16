#include "../include/headers.h"

/* Modify this file as needed*/
int remainingtime;

void SIGUSR1_Handler()
{
    // TODO Log when process halted
    //log(halt)
    //down - sem = 0
    //down - sem = -1 ->blocked
    //log(cont)
    // TODO Log when process continued
}

void SIGUSR2_Handler() {}

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
    // TODO Log when process started for the first time
    signal(SIGUSR1, SIGUSR1_Handler);
    signal(SIGUSR2, SIGUSR2_Handler);
    initClk();
    // TODO The process needs to get the remaining time from somewhere
    // remainingtime = ??;
    while (remainingtime > 0)
    {
        // remainingtime = ??;
    }

    destroyClk(false);

    return 0;
}
