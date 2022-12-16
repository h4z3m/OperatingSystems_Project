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
    ProcessControlBlock *newProcess = (ProcessControlBlock *)malloc(
        sizeof(ProcessControlBlock));
    newProcess->inputPID = pid;
    newProcess->arrivalTime = arrivalTime;
    newProcess->executionTime = newProcess->remainingTime = executionTime;
    // newProcess->memSize = memSize;
    newProcess->priority = priority;
    return newProcess;
}

/**
 * @brief
 *
 * @param fileName
 * @param processInfoArray
 */
void readInputFile(char *fileName, ProcessControlBlock *processInfoArray[])
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

    if (NULL == ptr)
    {
        DEBUG_PRINTF("[PROCGEN]File can't be opened.\n");
        return;
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
                    processInfoArray[i] = createProcess(id, arrival, runtime, input);

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

    return 0;
}

int calculateAverageWeightedTATime(ProcessControlBlock *pcb)
{
    return ((pcb->finishTime - pcb->arrivalTime) / pcb->executionTime);
}

int calculateAverageWaitingTime(ProcessControlBlock *processInfoArray[])
{
    // how to get size
    int size = 100;
    int sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum = sum + ((processInfoArray[i]->finishTime - processInfoArray[i]->arrivalTime) - processInfoArray[i]->executionTime);
    }
    return sum / size;
}

void sendMessage(ProcessControlBlock *pcbToSend)
{
    struct msgbuf buff;
    buff.mtype = 1;
    memcpy(&buff.pcb, pcbToSend, sizeof(ProcessControlBlock));
    int ret = msgsnd(proc_msgq, &buff, sizeof(struct msgbuf) - sizeof(long), 0);
    if (ret == -1)
    {
        perror("[PROCGEN] Error in message queue.");
        exit(-1);
    }
}

int main(int argc, char *argv[])
{

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    signal(SIGINT, clearResources);

    int process_count;
    int RR_quantum = 0;
    char *process_filename = argv[1];
    if (argc == 6)
        RR_quantum = atoi(argv[5]);

    if (-1 == (proc_msgq = msgget(MSGQKEY, IPC_CREAT | 0666)))
    {
        perror("[PROCGEN] Could not get message queue...\n");
        exit(-1);
    }

    DEBUG_PRINTF("[PROCGEN] Message queue ID = %d\n", proc_msgq);
    // TODO Initialization
    // 1. Read the input files.
    ProcessControlBlock **process_array = (ProcessControlBlock **)malloc(sizeof(ProcessControlBlock *) * 100);
    readInputFile(process_filename, process_array);
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    SchedulingAlgorithm_t sched = getSchedulerAlgorithm(*argv[3]) + '0';
    // 3. Initiate and create the scheduler and clock processes.
    // Create clk process
    int clk_pid = spawn_process("./clk.out", NULL);

    // Create scheduler process
    char sched_args[] = {sched, RR_quantum, '\0'};
    int scheduler_pid = spawn_process("./clk.out", &sched_args);

    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int prevClk = getClk();
    printf("Current Time is %d\n", prevClk);

    // 5. Create a data structure for processes and provide it with its parameters.

    // 6. Send the information to the scheduler at the appropriate time.
    struct msgbuf buff;
    buff.mtype = 1;
    for (;;)
    {
        int currClk = getClk();
        if (currClk != prevClk)
        {
            DEBUG_PRINTF("[PROCGEN] Current Time is %d\n", currClk);

            for (int i = 0; i < 10; i++)
            {
                if (process_array[i]->arrivalTime == currClk)
                {
                    // send to sched
                    DEBUG_PRINTF("[PROCGEN] Process[%d] arrived at %d\n", process_array[i]->inputPID, currClk);
                    sendMessage(process_array[i]);
                }
            }

            // // Signal scheduler about the new messages in queue
            // kill(scheduler_pid, SIGUSR1);

            // Update prev clk
            prevClk = getClk();
        }
    }
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    destroyClk(true);
    msgctl(proc_msgq, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
