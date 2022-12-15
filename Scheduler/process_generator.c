#include "headers.h"

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
ProcessControlBlock *createProcess(
    int pid,
    int arrivalTime,
    int executionTime,
    int memSize)
{
    ProcessControlBlock *newProcess = (ProcessControlBlock *)malloc(
        sizeof(ProcessControlBlock));
    newProcess->PID = pid;
    newProcess->arrivalTime = arrivalTime;
    newProcess->executionTime = executionTime;
    newProcess->memSize = memSize;
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
        printf("file can't be opened \n");
        return;
    }

    printf("content of this file are \n");

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
 * @param algorithmName
 * @return SchedulingAlgorithm_t
 */
SchedulingAlgorithm_t getSchedulerAlgorithm(char *algorithmName)
{
    if (strcmp(algorithmName, "SJF") == 0)
        return SchedulingAlgorithm_SJF;

    if (strcmp(algorithmName, "HPF") == 0)
        return SchedulingAlgorithm_HPF;

    if (strcmp(algorithmName, "RR") == 0)
        return SchedulingAlgorithm_RR;

    if (strcmp(algorithmName, "MLFL") == 0)
        return SchedulingAlgorithm_MLFL;

    return 0;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock.
    initClk();
    // To get time use this function.
    int x = getClk();
    printf("Current Time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
}
