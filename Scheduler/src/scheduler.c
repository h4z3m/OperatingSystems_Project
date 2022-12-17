#include "../include/data_structures.h"
#include "../include/headers.h"
#include "../include/types.h"
#include <math.h>
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

/* Message queue ID between scheduler and process generator*/
static int proc_msgq = 0;

/* Queue for output messages in scheduler.log*/
static Queue *outputQueue = NULL;

static Queue *finishedProcesses = NULL;
/* Represents the scheduler type which is to run */
static SchedulingAlgorithm_t schedAlgorithm = 0;

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

void saveProcessState(ProcessControlBlock *pcb, int remainingTime, int priority, ProcessState state, int finishTime)
{
    pcb->remainingTime = remainingTime;
    pcb->priority = priority;
    pcb->state = state;
    pcb->finishTime = finishTime;
}

void printProcessInfo(ProcessControlBlock *pcb)
{
    DEBUG_PRINTF("********\n");
    DEBUG_PRINTF("PID=%d\nArrival Time=%d\nExecution Time=%d\nPriority=%d\nMemsize=%d\n",
                 pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->priority, pcb->memSize);
    DEBUG_PRINTF("********\n");
}

int receiveMessage(ProcessControlBlock **pcbToGet)
{
    struct msgbuf newMsg;
    *pcbToGet = (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
    int ret = msgrcv(proc_msgq, (struct msgbuf *)&newMsg, sizeof(struct msgbuf) - sizeof(long), 0, IPC_NOWAIT);
    if (ret != -1)
    {
        // *mtype = newMsg.mtype;
        memcpy(*pcbToGet, &newMsg.pcb, sizeof(ProcessControlBlock));
    }
    return ret;
}

void enq_processStartedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = malloc(sizeof(char) * 200);
    int waitingTime = (pcb->startTime - pcb->arrivalTime);
    snprintf(buffer, 200, "At time %d process %d started arr %d total %d remain %d wait %d\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime, waitingTime);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void enq_processStoppedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = malloc(sizeof(char) * 100);
    snprintf(buffer, 100, "At time %d process %d stopped arr %d total %d remain %d wait 0\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void enq_processResumedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = malloc(sizeof(char) * 100);
    int waitingTime = currTime - pcb->arrivalTime - pcb->remainingTime;
    snprintf(buffer, 100, "At time %d process %d resumed arr %d total %d remain %d wait %d\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime, waitingTime);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void enq_processFinishedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = malloc(sizeof(char) * 100);
    int waitingTime = (currTime - pcb->arrivalTime) - pcb->executionTime;
    int TA = (pcb->finishTime - pcb->arrivalTime);
    float WTA = roundf(((float)TA / pcb->executionTime) * 100.0) / 100.0;
    snprintf(buffer, 100, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime, waitingTime, TA, WTA);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void logScheduler()
{
    FILE *logfile = fopen(SCHEDULER_LOG_FILENAME, "w");
    char *line = NULL;
    while (dequeue(outputQueue, &line))
    {
        fputs(line, logfile);
    }
    fclose(logfile);
    exit(0);
}

//

void scheduler_HPF()
{
    PriorityQueue *HPF_Queue = createPriorityQueue();
    // enqueuePriority(HPF_Queue, , );
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
    RR_Queue = createCircularQueue();
    finishedProcesses = createQueue();

    ProcessControlBlock *currentPCB = NULL;
    ProcessControlBlock *newPCB = NULL;

    int prevClk = -1;
    int quantum_passed = 0;
    // int mtype = -1;
    while (1)
    {

        prevClk = getClk();
        
        printf("[SCHEDULER] Current time = %d\n", prevClk);
        if (!circularIsEmpty(RR_Queue))
            quantum_passed++;

        /* New process has just arrived, put it at the back of queue*/
        while (receiveMessage(&newPCB) != -1)
        {

            enqueueCircular(RR_Queue, newPCB);
            newPCB->state = ProcessState_Ready;
            /* Queue was empty, */
            if (currentPCB == NULL)
            {
                currentPCB = newPCB;
                currentPCB->startTime = prevClk;
                saveProcessState(currentPCB, currentPCB->remainingTime, currentPCB->priority, ProcessState_Running, 0);
                enq_processStartedStr(prevClk, currentPCB);
            }
        }

        if (circularIsEmpty(RR_Queue))
            idle_cycles++;
        else
        {
            /* There is currently a process in execution, decrement its remaining time*/
            currentPCB->remainingTime--;

            // If process has just finished
            if (currentPCB->remainingTime == 0)
            {
                printf("AAAAAAAAAAAAAAA\n");
                /* Reset quantum passed as we will start a new process */
                quantum_passed = 0;

                saveProcessState(currentPCB, 0, currentPCB->priority, ProcessState_Finished, prevClk);

                enq_processFinishedStr(prevClk, currentPCB);

                /* Remove the process from the queue */
                dequeueCircularFront(RR_Queue, &currentPCB);

                enqueue(finishedProcesses, currentPCB);

                if (!peekFront(RR_Queue, &currentPCB))
                {
                    /* No more processes for now*/
                    currentPCB = NULL;
                }
                else
                {
                    if (currentPCB && currentPCB->inputPID == 3)
                        ;
                    /* Get next process and run it */
                    // Process is starting for the first time
                    if (currentPCB->state == ProcessState_Ready)
                    {
                        currentPCB->startTime = prevClk;
                        enq_processStartedStr(prevClk, currentPCB);
                    }
                    else
                        enq_processResumedStr(prevClk, currentPCB);

                    currentPCB->state = ProcessState_Running;
                }
            }
        }

        /* 1 quantum has just passed, schedule next process */
        if (quantum_passed == quantum)
        {
            if (!circularIsEmpty(RR_Queue))
            {
                ProcessControlBlock *oldProc = NULL;

                /* Remove from queue then enqueue so it's at the back */
                saveProcessState(currentPCB, currentPCB->remainingTime, currentPCB->priority, ProcessState_Blocked, 0);
                enq_processStoppedStr(prevClk, currentPCB);

                /* Remove the process from the queue */
                dequeueCircularFront(RR_Queue, &oldProc);

                enqueueCircular(RR_Queue, oldProc);

                /* Get new process */
                dequeueCircularFront(RR_Queue, &currentPCB);
                saveProcessState(currentPCB, currentPCB->remainingTime, currentPCB->priority, ProcessState_Running, 0);

                // Process is starting for the first time
                if (currentPCB->startTime == ProcessState_Ready)
                {
                    currentPCB->startTime = prevClk;
                    enq_processStartedStr(prevClk, currentPCB);
                }
                else
                    enq_processResumedStr(prevClk, currentPCB);
            }
            /* Reset quantum passed as we will start a new process */
            quantum_passed = 0;
        }

        while (getClk() == prevClk)
            ;
    }
    DEBUG_PRINTF("FINISHED\n");
}

void scheduler_MLFL() {}

void newProcess_Handler(int signum)
{
}

void processFinished_Handler(int signum) {}

void SIGINT_Handler(int signum)
{
    // TODO free all resources
    logScheduler();
    destroyClk(true);
    exit(0);
}

void getProcessMessageQueue()
{

    if (-1 == (proc_msgq = msgget(MSGQKEY, IPC_CREAT | 0666)))
    {
        perror("[SCHEDULER] Could not get message queue...");
        exit(-1);
    }
    else
    {
        DEBUG_PRINTF("[SCHEDULER] Message queue ID = %d\n", proc_msgq);
    }
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
    getProcessMessageQueue();
    outputQueue = createQueue();

    scheduler_RR(4);

    sleep(1);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    signal(SIGINT, SIGINT_Handler);
    signal(SIGUSR1, newProcess_Handler);

    DEBUG_PRINTF("[SCHEDULER] Scheduler started...\n");

    // Initialize output queue
    schedAlgorithm = atoi(argv[1]);
    int RR_quantum = atoi(argv[2]);
    // printf("RR = %d,Sched=%d", RR_quantum, schedAlgorithm);

    // schedAlgo = *argv[1];
    // TODO: implement the scheduler.
    switch (schedAlgorithm)
    {
    case SchedulingAlgorithm_HPF:
        scheduler_HPF();
        break;
    case SchedulingAlgorithm_RR:
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
