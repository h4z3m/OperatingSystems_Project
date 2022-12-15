#include "headers.h"

//  جاوب علي هذه الاسئلة و سوف تتصل بك هيفاء وهبي
// TODO Handlers for each scheduling algorithm or universal handler?
// TODO Who decrements process remaining time?
// TODO Who signals process that it's finished? Itself or scheduler?
// TODO How does the scheduler see new messages data? How can the sched. access the buffer?
// TODO In RR, process finished before its quantum is finished??? wat????
// TODO In RR, هكرم الضيف ازاي؟؟؟؟ when new process arrives at the same time a process finishes its quantum
// TODO Think of mlvl queue
// TODO Output lines to print when process starts/stops.. when to log?

/* Represents the idle cycles, must be updated somewhere */
static int idle_cycles = 0;

/* Represents the scheduler type which is to run */
static SchedulingAlgorithm_t schedAlgo = 0;

static PriorityQueue *HPF_Queue = NULL;
static PriorityQueue *SJF_Queue = NULL;
static MultiLevelQueue *MLVL_Queue = NULL;
static CircularQueue *RR_Queue = NULL;

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

// new process handler
void SIGUSR1_Handler()
{
    // stop current process
    // read all new processes using msgrcv
    // save in queue
    // decide which process to start (sched algo)
}

void SIGUSR2_Handler() {}

void scheduler_HPF()
{
    PriorityQueue *HPF_Queue = createPriorityQueue();
    enqueuePriority(HPF_Queue, , );
    // current executing function PID
    // get first element from queue (PID)
    // for each in enqueue
    // check if PID changed
    // current exec pid = first element from queue
    // signal old process to stop
    // signal new process to start
}

void scheduler_SJF()
{
    // current executing function PID
    // get first element from queue (PID)
    // for each in enqueue
    // current exec pid = first element from queue
    // signal old process to stop
    // signal new process to start
}
void scheduler_RR(unsigned int quantum)
{
    // current executing pid
    // for each clk cycle
    //  if current executing process rem time = 0
    // stop process
    // current executing = peek next from queue
    // if next quantum
    // signal old process to stop
    // signal new process to start
}

/**
 * @brief The scheduler which schedules which process needs to run and which to suspend.
 *
 * @param argc Argument count
 * @param argv Argument vector: Contains the scheduler algorithm type and optional params.
 * @return int
 */
int main(int argc, char *argv[])
{
    initClk();

    // TODO: implement the scheduler.
    switch (schedAlgo)
    {
    case SchedulingAlgorithm_HPF:
        signal(SIGUSR1, SIGUSR1_Handler);
        signal(SIGUSR2, SIGUSR2_Handler);
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
