#include "../include/headers.h"
#include "types.h"
#include <string.h>

/* Message queue ID between scheduler and process generator*/
static int proc_msgq = 0;

void clearResources(int);

/**
 * @brief Create a Process object
 *
 * @param pid
 * @param arrivalTime
 * @param executionTime
 * @param memSize
 * @return ProcessControlBlock*
 */
ProcessControlBlock *createProcess(int pid, int arrivalTime, int executionTime /*,int memSize*/, int priority)
{
    ProcessControlBlock *newProcess = (ProcessControlBlock *)calloc(
        1, sizeof(ProcessControlBlock));
    newProcess->inputPID = pid;
    newProcess->arrivalTime = arrivalTime;
    newProcess->executionTime = newProcess->remainingTime = executionTime;
    // newProcess->memSize = memSize;
    newProcess->priority = priority;
    newProcess->state = ProcessState_Ready;
    return newProcess;
}

/**
 * @brief
 *
 * @param fileName
 * @param processInfoArray
 */
int readInputFile(char *fileName, ProcessControlBlock *processInfoArray[])
{
    FILE *ptr = fopen(fileName, "r");
    char ch;
    int ch_;
    bool isReadingComment = false;
    int states = 0;
    int inc = 1;
    int input = 0;
    int i = -1;

    int id = 0;
    int arrival, runtime, priority;
    int processCount = 0;

    if (NULL == ptr)
    {
        DEBUG_PRINTF("[PROCGEN] File can't be opened.\n");
        exit(-1);
    }

    do
    {
        ch = fgetc(ptr);
        ch_ = ch;

        if (ch == '#')
        {
            isReadingComment = true;
        }

        if (!isReadingComment)
        {
            if (ch_ == 32 || ch_ == 9)
            {
                inc = 1;

                switch (states)
                {
                case 0:
                    id = input;
                    break;
                case 1:
                    arrival = input;
                    break;
                case 2:
                    runtime = input;
                    break;
                case 3:
                    priority = input;
                    break;
                }
                input = 0;
                states++;
            }
            else if (ch_ == 10)
            {
                if (i != -1)
                {
                    processInfoArray[i] = createProcess(id, arrival, runtime, input);
                    processCount++;
                }
                isReadingComment = false;
                states = 0;
                i++;
                input = 0;
                inc = 1;

                id = priority = arrival = runtime = 0;
            }
            else
            {

                input = input * inc + (ch - 48);

                inc = inc * 10;
            }
        }

        if (ch_ == 10 && i == -1)
        {
            isReadingComment = false;
            i++;
        }

    } while (ch != EOF);

    fclose(ptr);
    return processCount;
}

/**
 * @brief Get the Scheduler Algorithm object
 *
 * @param algorithmNum
 * @return SchedulingAlgorithm_t
 */
SchedulingAlgorithm_t getSchedulerAlgorithm(char algorithmNum)
{
    if (algorithmNum == '1')
        return SchedulingAlgorithm_SJF;

    if (algorithmNum == '2')
        return SchedulingAlgorithm_HPF;

    if (algorithmNum == '3')
        return SchedulingAlgorithm_RR;

    if (algorithmNum == '4')
        return SchedulingAlgorithm_MLFL;

    return SchedulingAlgorithm_HPF;
}


int sendMessage(ProcessControlBlock *pcbToSend)
{
    struct msgbuf buff = {0};
    buff.mtype = 1;
    // memcpy(&buff.pcb, pcbToSend, sizeof(ProcessControlBlock));
    buff.pcb.arrivalTime = pcbToSend->arrivalTime;
    buff.pcb.inputPID = pcbToSend->inputPID;
    buff.pcb.executionTime = pcbToSend->executionTime;
    buff.pcb.remainingTime = pcbToSend->remainingTime;
    buff.pcb.state = pcbToSend->state;
    buff.pcb.memSize = pcbToSend->memSize;
    buff.pcb.priority = pcbToSend->priority;

    int ret = msgsnd(proc_msgq, &buff, sizeof(struct msgbuf) - sizeof(long), !IPC_NOWAIT);
    if (ret == -1)
    {
        perror("[PROCGEN] Error in message queue.");
        exit(-1);
    }
    return ret;
}

int main(int argc, char *argv[])


{

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    signal(SIGINT, clearResources);

    int RR_quantum;
    char *process_filename = argv[1];
    if (argc == 6)
        RR_quantum = atoi(argv[5]);

    if (-1 == (proc_msgq = msgget(MSGQKEY, IPC_CREAT | 0666)))
    {
        perror("[PROCGEN] Could not get message queue...\n");
        exit(-1);
    }

    DEBUG_PRINTF("[PROCGEN] Message queue ID = %d\n", proc_msgq);

    // 1. Read the input files.
    ProcessControlBlock **process_array = (ProcessControlBlock **)malloc(sizeof(ProcessControlBlock *) * 100);
    int process_count = readInputFile(process_filename, process_array);

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    SchedulingAlgorithm_t sched = getSchedulerAlgorithm(*argv[3]);

    // 3. Initiate and create the scheduler and clock processes.
    // Create clk process

    // Create scheduler process
    char schede[5] = {0};
    char quantum[5] = {0};
    char *schedexec = "./scheduler.out";
    // sprintf(schede, "%d", sched);
    // sprintf(quantum, "%d", RR_quantum);

    sprintf(schede, "1");
    sprintf(quantum, "%d", RR_quantum);

    char *args[] = {schedexec, schede, quantum, NULL};

    int clk_pid = fork();
    if (clk_pid == 0)
        execv("./clk.out", NULL);

    int scheduler_pid = fork();
    if (scheduler_pid == 0)
        execv(args[0], args);

    // 4. Use this function after creating the clock process to initialize clock.

    initClk();

    // To get time use this function.
    int prevClk = getClk();
    printf("Current Time is %d\n", prevClk);

    // 5. Create a data structure for processes and provide it with its parameters.

    // 6. Send the information to the scheduler at the appropriate time.
    int processes_sent = 0;
    for (;;)
    {
        int currClk = getClk();
        if (currClk != prevClk)
        {
            prevClk = getClk();
            DEBUG_PRINTF("[PROCGEN] Current Time is %d\n", getClk());

            for (int i = 0; i < process_count; i++)
            {
                /* code */
                if (process_array[i]->arrivalTime == prevClk+1)
                {
                    // send to sched
                    DEBUG_PRINTF("[PROCGEN] Process[%d] arrived at %d\n", process_array[i]->inputPID, prevClk);
                    
                    sendMessage(process_array[i]);
                    // DEBUG_PRINTF("[PROCGEN] after sending %d\n", getClk());
                    processes_sent++;
                }
                // if (processes_sent == process_count)
                // {
                //     raise(SIGINT);
                // }
            }
            currClk = getClk();
        }
        // 7. Clear clock resources
    }
    destroyClk(true);
}
void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    destroyClk(true);
    msgctl(proc_msgq, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
