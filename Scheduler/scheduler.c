#include "headers.h"
/* Represents the idle cycles, must be updated somewhere */
static int idle_cycles = 0;

/* Represents the scheduler type which is to run */
static SchedulingAlgorithm_t schedAlgo = 0;

/**
 * @brief
 *
 * @param pcb
 * @return int
 */
int calculateAverageWeightedTATime(ProcessControlBlock *pcb);
/**
 * @brief
 *
 * @param processInfoArray
 * @return int
 */
int calculateAverageWaitingTime(ProcessControlBlock *processInfoArray[]);

/**
 * @brief
 *
 * @param processInfoArray
 */
void generateSchedulerLog(ProcessControlBlock *processInfoArray[]);
/**
 * @brief
 *
 * @param processInfoArray
 */
void generateSchedulerPerf(ProcessControlBlock *processInfoArray[]);

/**
 * @brief
 *
 * @param pcb
 */
void saveProcessState(ProcessControlBlock *pcb, int remainingTime, int priority, ProcessState state)
{
    pcb->remainingTime = remainingTime;
    pcb->priority = priority;
    pcb->state = state;
}

void scheduler_RR(char quantum);
void scheduler_HPF();
void scheduler_SJF();
void scheduler_MLFL();

/**
 * @brief The scheduler which schedules which process needs to run and which to suspend.
 *
 * @param argc Argument count
 * @param argv Argument vector: Contains the scheduler algorithm type and optional params.
 * @return int
 */
int main(int argc, char *argv[])
{
    signal(SIGUSR1, SIGUSR1_Handler);
    signal(SIGUSR2, SIGUSR2_Handler);
    initClk();

    // TODO: implement the scheduler.
    switch (schedAlgo)
    {
    case SchedulingAlgorithm_HPF:
        scheduler_HPF();
        break;
    case SchedulingAlgorithm_RR:
        scheduler_RR(1);
        break;
    case SchedulingAlgorithm_SJF:
        scheduler_SJF();
        break;
    case SchedulingAlgorithm_MLFL:
        scheduler_MLFL();
        break;
    }

    // TODO: upon termination release the clock resources.

    destroyClk(true);
}

void SIGUSR1_Handler();
void SIGUSR2_Handler();
