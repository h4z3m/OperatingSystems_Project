#include "../include/headers.h"
#include "types.h"
#include <string.h>

/* Message queue ID between scheduler and process generator*/
static int proc_msgq = 0;

void clearResources(int);
/**
 * @brief Creates a new process with given data, and returns a pointer to it.
 *
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
 * @brief De-allocates the process array along with the processes inside it
 *
 * @param array Process array
 * @param count Processes count
 */
void destroyProcessArray(ProcessControlBlock **array, int count)
{
    ProcessControlBlock *pcb;
    for (int i = 0; i < count; i++)
    {
        pcb = array[i];
        free(pcb);
    }
    free(array);
}

/**
 * @brief Reads the input file which contains the processes
 *
 * @param fileName          Filename
 * @param processInfoArray  Array which will contain new processes
 * @return int              Process count which was read from file
 */
int readInputFile(char *fileName, ProcessControlBlock ***processInfoArray)
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
        perror("[PROCGEN] Input file can't be opened.\n");
        exit(-1);
    }

    /* Count the number of processes, skip comments*/
    char c;
    for (c = getc(ptr); c != EOF; c = getc(ptr))
        if (c == '\n')
            processCount++;
        else if (c == '#')
            while (c = fgetc(ptr), c != '\n' && c != EOF)
                ;
    rewind(ptr);
    *processInfoArray = (ProcessControlBlock **)malloc(sizeof(ProcessControlBlock *) * processCount);

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
                    (*processInfoArray)[i] = createProcess(id, arrival, runtime, input);
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
 * @param algorithmNum              Algorithm number as character
 * @return SchedulingAlgorithm_t    Scheduling algorithm type
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

/**
 * @brief   Sends a message to the scheduler with the given process.
 *
 * @param pcbToSend Pointer to process to send
 * @param mtype     Message type
 * @return int      Status of sending operation in message queue
 * @return -1       Fail/Error
 * @return !=-1     Success
 */
int sendMessage(ProcessControlBlock *pcbToSend, long mtype)
{
    struct msgbuf buff = {0};
    buff.mtype = mtype;
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

/**
 * @brief   Initializes the message queue between the scheduler and process generator
 *
 */
void initMessageQueue()
{
    if (-1 == (proc_msgq = msgget(MSGQKEY, IPC_CREAT | 0666)))
    {
        perror("[PROCGEN] Could not get message queue...\n");
        exit(-1);
    }

    DEBUG_PRINTF("[PROCGEN] Message queue ID = %d\n", proc_msgq);
}

/**
 * @brief
 *
 * @param argc  Argument count
 * @param argv  Argument vector
 *        argv[1]:  Path to processes input file
 *        argv[2]:  "-sch"  Scheduler ID opt
 *        argv[3]:  Algorithm number (1-4)
 *        argv[4]:  "-q"    Quantum time opt
 *        argv[5]:  Quantum time for RR or MLFL if they were selected
 * @return int
 */
int main(int argc, char *argv[])
{
    /* Sets stdout & stderr to be unbuffered -
        to output at any time without needing \n character
    */
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Attach signal handler to SIGINT*/
    signal(SIGINT, clearResources);

    int RR_quantum;
    /* Get input filename from input arguments*/
    char *process_filename = argv[1];
    if (argc == 6)
        RR_quantum = atoi(argv[5]);

    // 1. Read the input files.

    ProcessControlBlock **process_array = NULL;

    int process_count = readInputFile(process_filename, &process_array);
    DEBUG_PRINTF("[PROCGEN] Processes = %d\n", process_count);
    initMessageQueue();

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.

    SchedulingAlgorithm_t sched = getSchedulerAlgorithm(*argv[3]);

    // 3. Initiate and create the scheduler and clock processes.

    /* Argument buffers */
    char schede[5] = {0};
    char quantum[5] = {0};
    char proc_count[5] = {0};
    char *schedexec = (char *)"./scheduler.out";

    // sprintf(schede, "4");
    // sprintf(quantum, "%d", 4);

    /* Format arguments as character numbers */
    sprintf(schede, "%d", sched);
    sprintf(quantum, "%d", RR_quantum);
    sprintf(proc_count, "%d", process_count);
    char *schedargs[] = {schedexec, schede, quantum, proc_count, NULL};

    // Create clk process

    /* Argument buffers */
    char clkexec[] = "clk.out";
    char *clkargs[] = {clkexec, NULL};
    pid_t clk_pid = fork();
    if (clk_pid == 0)
        execv(clkargs[0], clkargs);

    // Create scheduler process
    pid_t scheduler_pid = fork();
    if (scheduler_pid == 0)
        execv(schedargs[0], schedargs);

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
                /* Send a process at its arrival time */
                if (process_array[i]->arrivalTime == prevClk + 1)
                {
                    /* Send to sched */
                    DEBUG_PRINTF("[PROCGEN] Process[%d] arrived at %d\n", process_array[i]->inputPID, prevClk + 1);
                    sendMessage(process_array[i], 1);
                    processes_sent++;
                }
                /* Generator has sent all processes */
                if (processes_sent == process_count)
                {
                    pid_t pid;
                    /* Wait for scheduler */
                    do
                    {
                        int ret = 0;
                        pid = wait(&ret);

                    } while (pid != scheduler_pid);
                    // raise(SIGINT);
                    goto CLEAR_RESOURCES;
                }
                currClk = getClk();
            }
        }
    }
CLEAR_RESOURCES:
    // 7. Clear clock resources
    clearResources(0);
}

/**
 * @brief   - Clears the resources used by the process generator.
 *          - Destroys the clock and signals all child processes to terminate.
 *          - Destroys the message queue between scheduler and process generator
 *          - Frees the processes array
 * @param signum    Signal number
 */
void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
    DEBUG_PRINTF("[PROCGEN] Terminating...\n");

    destroyClk(true);

    msgctl(proc_msgq, IPC_RMID, (struct msqid_ds *)0);
    exit(0);
}
