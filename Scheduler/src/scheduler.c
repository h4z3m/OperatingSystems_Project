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

/* Represents the idle cycles, must be updated somewhere */
static int total_cycles = 0;

/* Message queue ID between scheduler and process generator*/
static int proc_msgq = 0;

/* Queue for output messages in scheduler.log*/
static Queue *outputQueue = NULL;

static Queue *finishedProcesses = NULL;

/* Represents the scheduler type which is to run */
static SchedulingAlgorithm_t schedAlgorithm = SchedulingAlgorithm_RR;

static PriorityQueue *HPF_Queue = NULL;
static PriorityQueue *SJF_Queue = NULL;
static MultiLevelQueue *MLVL_Queue = NULL;
static CircularQueue *RR_Queue = NULL;

int calculateAverageWeightedTATime(ProcessControlBlock *pcb)
{
    return ((pcb->finishTime - pcb->arrivalTime) / pcb->executionTime);
}

int calculateAverageWaitingTime(Queue *processInfoQueue, int size)
{
    int sum = 0;
    ProcessControlBlock *pcb;
    while (dequeue(processInfoQueue, &pcb))
    {

        sum = sum + ((pcb->finishTime - pcb->arrivalTime) - pcb->executionTime);
    }
    return sum / size;
}
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
void generateSchedulerPerf(Queue *processInfoArray[]);

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
        // memcpy(*pcbToGet, &newMsg.pcb, sizeof(ProcessControlBlock));
        (*pcbToGet)->arrivalTime = newMsg.pcb.arrivalTime;

        (*pcbToGet)->inputPID = newMsg.pcb.inputPID;
        (*pcbToGet)->executionTime = newMsg.pcb.executionTime;
        (*pcbToGet)->remainingTime = newMsg.pcb.remainingTime;
        (*pcbToGet)->state = newMsg.pcb.state;
        (*pcbToGet)->memSize = newMsg.pcb.memSize;
        (*pcbToGet)->priority = newMsg.pcb.priority;
    }
    return ret;
}

void output_processStartedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = (char *)malloc(sizeof(char) * 100);
    int waitingTime = (pcb->startTime - pcb->arrivalTime);
    snprintf(buffer, 100, "At time\t%d\tprocess\t%d\tstarted arr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime, waitingTime);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void output_processStoppedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = (char *)malloc(sizeof(char) * 100);
    snprintf(buffer, 100, "At time\t%d\tprocess\t%d\tstopped arr\t%d\ttotal\t%d\tremain\t%d\twait\t0\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void output_processResumedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = (char *)malloc(sizeof(char) * 100);
    int waitingTime = currTime - pcb->arrivalTime - pcb->remainingTime;
    snprintf(buffer, 100, "At time\t%d\tprocess\t%d\tresumed arr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime, waitingTime);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void output_processFinishedStr(int currTime, ProcessControlBlock *pcb)
{
    char *buffer = (char *)malloc(sizeof(char) * 100);
    int waitingTime = (pcb->finishTime - pcb->arrivalTime) - pcb->executionTime;
    int TA = (pcb->finishTime - pcb->arrivalTime);
    float WTA = roundf(((float)TA / pcb->executionTime) * 100.0) / 100.0;
    snprintf(buffer, 100, "At time\t%d\tprocess\t%d\tfinished arr\t%d\ttotal\t%d\tremain\t%d\twait\t%d\tTA\t%d WTA\t%.2f\n",
             currTime, pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->remainingTime, waitingTime, TA, WTA);
    enqueue(outputQueue, buffer);
    DEBUG_PRINTF("%s", buffer);
}

void logScheduler()
{
    FILE *logfile = fopen(SCHEDULER_LOG_FILENAME, "w");
    char *line = NULL;
    while (dequeue(outputQueue, (void **)&line))
    {
        fputs(line, logfile);
    }
    fclose(logfile);
    exit(0);
}

void logPerfScheduler()
{
    float utilizationPercent = ((float)idle_cycles / total_cycles) * 100;
    float WTA = 0;
}

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

    SJF_Queue = createPriorityQueue();
    ProcessControlBlock *currentPCB = (NULL);
    ProcessControlBlock *newPCB = (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
    ProcessControlBlock *frontPCB = NULL;
    int currClk = -1;

    for (;;)
    {

        currClk = getClk();
        DEBUG_PRINTF("[SCHED] Current Time is %d\n", currClk);

        /* New process has just arrived, put it in queue, then check if it has lower rem time than current*/
        while (receiveMessage(&newPCB) != -1)
        {

            /* Enqueue new process, priority = execution time*/
            enqueuePriority(SJF_Queue, newPCB, (newPCB->executionTime) * (-1));
            newPCB->state = ProcessState_Ready;
            /* Queue was empty, */
            if (currentPCB == NULL)
            {
                currentPCB = newPCB;
                currentPCB->startTime = currClk;
                saveProcessState(currentPCB,
                                 currentPCB->remainingTime,
                                 currentPCB->priority,
                                 ProcessState_Running, 0);
                output_processStartedStr(currClk, currentPCB);
            }

        }

        if (currentPCB == NULL)
            idle_cycles++;
        else
        {

            if (currentPCB->remainingTime == 0)
            {
                /* Save process state and dequeue it*/
                saveProcessState(currentPCB,
                                 0,
                                 currentPCB->priority,
                                 ProcessState_Finished, currClk);
                output_processFinishedStr(currClk, currentPCB);
                removeNodePriority(SJF_Queue, (void **)&currentPCB);

                currentPCB = NULL;

                /* Get new process(if exists) from queue as the current is finished*/
                if (dequeuePriority(SJF_Queue, (void **)&currentPCB))
                {
                    currentPCB->startTime = currClk;
                    saveProcessState(currentPCB,
                                     currentPCB->remainingTime,
                                     currentPCB->priority,
                                     ProcessState_Running, 0);
                    output_processStartedStr(currClk, currentPCB);
                }
            }
            /* Decrement current process time */
            currentPCB->remainingTime--;
        }
        total_cycles++;
        while (currClk == getClk())
            ;
    }
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
    unsigned int quantum_passed = 0;
    // int mtype = -1;
    while (1)
    {
        while (getClk() == prevClk)
            ;

        prevClk = getClk();

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
                output_processStartedStr(prevClk, currentPCB);
            }
        }

        if (!circularIsEmpty(RR_Queue))
            quantum_passed++;

        if (circularIsEmpty(RR_Queue))
            idle_cycles++;
        else
        {
            /* There is currently a process in execution, decrement its remaining time*/
            currentPCB->remainingTime--;

            // If process has just finished
            if (currentPCB->remainingTime = 0)
            {
                /* Reset quantum passed as we will start a new process */
                quantum_passed = 0;

                saveProcessState(currentPCB, 0, currentPCB->priority, ProcessState_Finished, prevClk);

                output_processFinishedStr(prevClk, currentPCB);

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
                        output_processStartedStr(prevClk, currentPCB);
                    }
                    else
                        output_processResumedStr(prevClk, currentPCB);

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
                output_processStoppedStr(prevClk, currentPCB);

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
                    output_processStartedStr(prevClk, currentPCB);
                }
                else
                    output_processResumedStr(prevClk, currentPCB);
            }
            /* Reset quantum passed as we will start a new process */
            quantum_passed = 0;
        }
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
    DEBUG_PRINTF("[SCHEDULER] Scheduler started...\n");

    initClk();

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    signal(SIGINT, SIGINT_Handler);
    signal(SIGUSR1, newProcess_Handler);

    // Initialize output queue
    outputQueue = createQueue();
    getProcessMessageQueue();

    schedAlgorithm = (SchedulingAlgorithm_t)atoi(argv[1]);
    int RR_quantum;

    // TODO: implement the scheduler.
    switch (schedAlgorithm)
    {
    case SchedulingAlgorithm_HPF:
        scheduler_HPF();
        break;
    case SchedulingAlgorithm_RR:
        RR_quantum = atoi(argv[2]);
        scheduler_RR(RR_quantum);
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
