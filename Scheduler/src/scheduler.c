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

/* Message queue ID between scheduler and process generator */
static int proc_msgq = 0;

/* Queue for output messages in scheduler.log */
static Queue *outputQueue = NULL;

/* Queue for finished processes, used in logging */
static Queue *finishedProcesses = NULL;

/* Represents the scheduler type which is to run */
static SchedulingAlgorithm_t schedAlgorithm = SchedulingAlgorithm_RR;

/* Scheduler queues */
static PriorityQueue *HPF_Queue = NULL;
static PriorityQueue *SJF_Queue = NULL;
static MultiLevelQueue *MLVL_Queue = NULL;
static Queue *RR_Queue = NULL;

float calculateAverageWeightedTATime(Queue *processInfoQueue, int size)
{
    ProcessControlBlock *pcb;
    Node *ptr = processInfoQueue->front;
    float TA = 0;
    float RT = 0;
    float sum = 0;
    while (ptr)
    {
        pcb = ptr->dataPtr;
        TA = (pcb->finishTime - pcb->arrivalTime);
        RT = pcb->executionTime;
        sum = sum + (TA / RT);
        ptr = ptr->nextNode;
    }
    return sum / size;
}

float calculateAverageWaitingTime(Queue *processInfoQueue, int size)
{
    float sum = 0;
    ProcessControlBlock *pcb;
    Node *ptr = processInfoQueue->front;
    while (ptr)
    {
        pcb = ptr->dataPtr;
        sum = sum + ((pcb->finishTime - pcb->arrivalTime) - pcb->executionTime);
        ptr = ptr->nextNode;
    }
    return sum / size;
}

void generateSchedulerLog(Queue *lines);

void generateSchedulerPerf(Queue *processInfoQueue);

void saveProcessState(ProcessControlBlock *pcb, int remainingTime, int priority, ProcessState state, int finishTime)
{
    pcb->remainingTime = remainingTime;
    pcb->priority = priority;
    pcb->state = state;
    pcb->finishTime = finishTime;
}

void printProcessInfo(ProcessControlBlock *pcb)
{
    DEBUG_PRINTF("===================\n");
    DEBUG_PRINTF("PID=%d\nArrival Time=%d\nExecution Time=%d\nPriority=%d\nMemsize=%d\n",
                 pcb->inputPID, pcb->arrivalTime, pcb->executionTime, pcb->priority, pcb->memSize);
    DEBUG_PRINTF("===================\n");
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

void generateSchedulerLog(Queue *lines)
{
    FILE *logfile = fopen(SCHEDULER_LOG_FILENAME, "w");
    char *line = NULL;
    fputs("#At time x process y state arr w total z remain y wait k\n", logfile);
    while (dequeue(lines, (void **)&line))
    {
        fputs(line, logfile);
    }
    fclose(logfile);
}

void generateSchedulerPerf(Queue *processInfoQueue)
{
    /* Calculate */
    float utilizationPercent = ((float)idle_cycles / total_cycles) * 100;
    float WTA = calculateAverageWeightedTATime(processInfoQueue, processInfoQueue->capacity);
    float avgWaiting = calculateAverageWaitingTime(processInfoQueue, processInfoQueue->capacity);

    printf("\nCPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.1f",
           utilizationPercent,
           WTA,
           avgWaiting);

    /* Round */

    utilizationPercent = roundf(((float)utilizationPercent) * 100);
    WTA = roundf(((float)WTA) * 100);
    avgWaiting = roundf(((float)avgWaiting) * 100) / 100.0;

    /* Write to file */

    FILE *perfFile = fopen(SCHEDULER_PERF_FILENAME, "w");

    fprintf(perfFile, "CPU utilization = %.2f%%\nAvg WTA = %.2f\nAvg Waiting = %.1f",
            utilizationPercent,
            WTA,
            avgWaiting);

    fclose(perfFile);
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
    ProcessControlBlock *currentPCB = (NULL);
    ProcessControlBlock *newPCB = (ProcessControlBlock *)malloc(sizeof(ProcessControlBlock));
    int currClk = -1;

    for (;;)
    {

        currClk = getClk();
        DEBUG_PRINTF("[SCHED] Current Time is %d\n", currClk);

        /* New process has just arrived, put it in queue, then check if it has lower rem time than current*/
        while (receiveMessage(&newPCB) != -1)
        {

            /* Enqueue new process, priority = execution time*/
            enqueuePriority(HPF_Queue, newPCB, (newPCB->priority));
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
            else
            {
                // if I was less priority than the running process then save the status of the current and replace it
                if (newPCB->priority < currentPCB->priority)
                {
                    saveProcessState(currentPCB,
                                     currentPCB->remainingTime,
                                     currentPCB->priority,
                                     ProcessState_Blocked, 0);
                    output_processStoppedStr(currClk, currentPCB);
                    currentPCB = newPCB;
                    currentPCB->startTime = currClk;
                    saveProcessState(currentPCB,
                                     currentPCB->remainingTime,
                                     currentPCB->priority,
                                     ProcessState_Running, 0);
                    output_processStartedStr(currClk, currentPCB);
                }
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
                removeNodePriority(HPF_Queue, (void **)&currentPCB);

                currentPCB = NULL;

                /* Get new process(if exists) from queue as the current is finished*/
                if (peekPriority(HPF_Queue, (void **)&currentPCB))
                {
                    if (currentPCB->state == ProcessState_Blocked)
                    {
                        currentPCB->startTime = currClk;
                        saveProcessState(currentPCB,
                                         currentPCB->remainingTime,
                                         currentPCB->priority,
                                         ProcessState_Running, 0);
                        output_processResumedStr(currClk, currentPCB);
                    }
                    else
                    {
                        currentPCB->startTime = currClk;
                        saveProcessState(currentPCB,
                                         currentPCB->remainingTime,
                                         currentPCB->priority,
                                         ProcessState_Running, 0);
                        output_processStartedStr(currClk, currentPCB);
                    }
                }
            }
            /* Decrement current process time */
            if (currentPCB)
                currentPCB->remainingTime--;
        }
        total_cycles++;
        while (currClk == getClk())
            ;
    }
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
            enqueuePriority(SJF_Queue, newPCB, (newPCB->executionTime));
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

void scheduler_RR(int quantum)
{
    // current executing pid
    // for each clk cycle
    //  if current executing process rem time = 0
    // stop process
    // current executing = peek next from queue
    // if next quantum
    // signal old process to stop
    // signal new process to start
    RR_Queue = createQueue();
    finishedProcesses = createQueue();

    ProcessControlBlock *currentPCB = NULL;
    ProcessControlBlock *newPCB = NULL;

    int prevClk = -1;
    int quantum_passed = 0;
    // int mtype = -1;

    for (;;)
    {

        prevClk = getClk();
        DEBUG_PRINTF("[SCHED] Current Time is %d\n", prevClk);

        /* New process has just arrived, put it at the back of queue*/
        while (receiveMessage(&newPCB) != -1)
        {
            printProcessInfo(newPCB);
            enqueue(RR_Queue, newPCB);
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

        if (!currentPCB)
        {
            idle_cycles++;
        }
        else
        {
            // Interrupt process when:
            // Case 1: Finish on quantum
            // Case 2: Finished before quantum
            // Case 3: Didn't finish before quantum
            // Post quantum: schedule new process

            if (currentPCB->remainingTime == 0)
            {
                /* Reset quantum passed as we will start a new process */
                quantum_passed = 0;

                saveProcessState(currentPCB, 0,
                                 currentPCB->priority, ProcessState_Finished, prevClk);

                output_processFinishedStr(prevClk, currentPCB);

                /* Remove the process from the queue */
                dequeue(RR_Queue, (void **)&currentPCB);

                enqueue(finishedProcesses, &currentPCB);

                if (!peek(RR_Queue, (void **)&currentPCB))
                {
                    /* No more processes for now*/
                    currentPCB = NULL;
                }
                else
                {
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
            if (!isEmpty(RR_Queue) && RR_Queue->front->nextNode != NULL)
            {

                ProcessControlBlock *oldProc = NULL;

                /* Remove from queue then enqueue so it's at the back */
                saveProcessState(currentPCB, currentPCB->remainingTime, currentPCB->priority, ProcessState_Blocked, 0);
                output_processStoppedStr(prevClk, currentPCB);

                /* Remove the process from the queue */
                dequeue(RR_Queue, (void **)&oldProc);

                /* Check if process finished exactly on quantum */
                enqueue(RR_Queue, oldProc);

                /* Get new process */
                peek(RR_Queue, (void **)&currentPCB);

                // Process is starting for the first time
                if (currentPCB->state == ProcessState_Ready)
                {
                    currentPCB->startTime = prevClk;
                    output_processStartedStr(prevClk, currentPCB);
                }
                else
                    output_processResumedStr(prevClk, currentPCB);

                saveProcessState(currentPCB, currentPCB->remainingTime, currentPCB->priority, ProcessState_Running, 0);
            }
            /* Reset quantum passed as we will start a new process */
            quantum_passed = 0;
        }
        /* There is currently a process in execution, decrement its remaining time*/
        if (currentPCB)
        {
            currentPCB->remainingTime--;
            quantum_passed++;
        }
        total_cycles++;
        while (prevClk == getClk())
            ;
        if (prevClk == 15)
        {
            printf("AAAAAA");
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
    generateSchedulerLog(outputQueue);
    generateSchedulerPerf(finishedProcesses);
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
