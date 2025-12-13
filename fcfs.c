#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PROCESSES 50
#define HASH_SIZE 101
#define MAX_KILL_EVENTS 20

typedef enum {
    STATE_READY,
    STATE_RUNNING,
    STATE_WAITING,
    STATE_TERMINATED,
    STATE_KILLED
} ProcessState;

typedef struct ProcessControlBlock {
    char processName[20];
    int processId;
    int cpuBurstTime;
    int ioStartTime;
    int ioDuration;

    int executedTime;
    int remainingIoTime;
    int completionTime;

    ProcessState state;
    struct ProcessControlBlock *next;
} PCB;

typedef struct {
    int processId;
    int killTime;
} KillEvent;

typedef struct {
    PCB *front;
    PCB *rear;
} ProcessQueue;

PCB *processHashTable[HASH_SIZE];
KillEvent killEvents[MAX_KILL_EVENTS];
int killEventCount = 0;
int systemClock = 0;

int isValidInteger(const char *str) {
    if (*str == '\0') return 0;
    for (int i = 0; str[i]; i++) {
        if (!isdigit(str[i]) && !(i == 0 && str[i] == '-'))
            return 0;
    }
    return 1;
}

int isValidProcessName(const char *name) {
    if (*name == '\0') return 0;
    for (int i = 0; name[i]; i++) {
        if (!isalpha(name[i]) && name[i] != '_')
            return 0;
    }
    return 1;
}

int hashFunction(int processId) {
    return processId % HASH_SIZE;
}

PCB* findProcessById(int processId) {
    PCB *current = processHashTable[hashFunction(processId)];
    while (current) {
        if (current->processId == processId)
            return current;
        current = current->next;
    }
    return NULL;
}

void insertProcess(PCB *process) {
    int index = hashFunction(process->processId);
    process->next = processHashTable[index];
    processHashTable[index] = process;
}

void initializeQueue(ProcessQueue *queue) {
    queue->front = queue->rear = NULL;
}

int isQueueEmpty(ProcessQueue *queue) {
    return queue->front == NULL;
}

void enqueueProcess(ProcessQueue *queue, PCB *process) {
    process->next = NULL;
    if (!queue->rear) {
        queue->front = queue->rear = process;
    } else {
        queue->rear->next = process;
        queue->rear = process;
    }
}

PCB* dequeueProcess(ProcessQueue *queue) {
    if (isQueueEmpty(queue)) return NULL;
    PCB *process = queue->front;
    queue->front = process->next;
    if (!queue->front) queue->rear = NULL;
    process->next = NULL;
    return process;
}

void removeProcessFromQueue(ProcessQueue *queue, PCB *process) {
    PCB *current = queue->front;
    PCB *previous = NULL;

    while (current) {
        if (current == process) {
            if (previous)
                previous->next = current->next;
            else
                queue->front = current->next;

            if (current == queue->rear)
                queue->rear = previous;

            current->next = NULL;
            return;
        }
        previous = current;
        current = current->next;
    }
}

void updateWaitingProcesses(ProcessQueue *waitingQueue, ProcessQueue *readyQueue) {
    PCB *current = waitingQueue->front;
    PCB *previous = NULL;

    while (current) {
        current->remainingIoTime--;
        if (current->remainingIoTime == 0) {
            PCB *completedIoProcess = current;
            completedIoProcess->state = STATE_READY;

            if (previous)
                previous->next = current->next;
            else
                waitingQueue->front = current->next;

            if (current == waitingQueue->rear)
                waitingQueue->rear = previous;

            current = current->next;
            enqueueProcess(readyQueue, completedIoProcess);
        } else {
            previous = current;
            current = current->next;
        }
    }
}

void processKillEvents(int currentTime, ProcessQueue *readyQueue,
                       ProcessQueue *waitingQueue, PCB **runningProcess) {
    for (int i = 0; i < killEventCount; i++) {
        if (killEvents[i].killTime == currentTime) {
            PCB *process = findProcessById(killEvents[i].processId);
            if (!process || process->state == STATE_TERMINATED || process->state == STATE_KILLED)
                continue;

            process->state = STATE_KILLED;
            process->completionTime = currentTime;

            removeProcessFromQueue(readyQueue, process);
            removeProcessFromQueue(waitingQueue, process);

            if (*runningProcess == process)
                *runningProcess = NULL;
        }
    }
}

int main() {
    ProcessQueue readyQueue, waitingQueue;
    initializeQueue(&readyQueue);
    initializeQueue(&waitingQueue);

    PCB *allProcesses[MAX_PROCESSES];
    char inputBuffer[32];
    int processCount;

    printf("Enter number of processes: ");
    scanf("%s", inputBuffer);

    if (!isValidInteger(inputBuffer)) {
        printf("ERROR: Invalid process count\n");
        return 0;
    }

    processCount = atoi(inputBuffer);

    if (processCount <= 0 || processCount > MAX_PROCESSES) {
        printf("ERROR: Process count out of range\n");
        return 0;
    }

    for (int i = 0; i < processCount; i++) {
        PCB *process = malloc(sizeof(PCB));
        char pidStr[16], cpuStr[16], ioStartStr[16], ioDurStr[16];

        printf("Enter <name pid cpu_burst io_start io_duration>: ");
        scanf("%s %s %s %s %s",
              process->processName, pidStr, cpuStr, ioStartStr, ioDurStr);

        if (!isValidProcessName(process->processName)) {
            printf("ERROR: Invalid process name\n");
            return 0;
        }

        if (!isValidInteger(pidStr) || !isValidInteger(cpuStr) ||
            !isValidInteger(ioStartStr) || !isValidInteger(ioDurStr)) {
            printf("ERROR: Numeric fields must be integers\n");
            return 0;
        }

        process->processId = atoi(pidStr);
        process->cpuBurstTime = atoi(cpuStr);
        process->ioStartTime = atoi(ioStartStr);
        process->ioDuration = atoi(ioDurStr);

        if (findProcessById(process->processId)) {
            printf("ERROR: Duplicate process ID %d\n", process->processId);
            return 0;
        }

        if (process->cpuBurstTime <= 0 || process->ioDuration < 0) {
            printf("ERROR: Invalid CPU or IO values\n");
            return 0;
        }

        if (process->ioDuration > 0 &&
            (process->ioStartTime <= 0 || process->ioStartTime >= process->cpuBurstTime)) {
            printf("ERROR: Invalid IO start time\n");
            return 0;
        }

        process->executedTime = 0;
        process->remainingIoTime = process->ioDuration;
        process->state = STATE_READY;
        process->next = NULL;

        insertProcess(process);
        enqueueProcess(&readyQueue, process);
        allProcesses[i] = process;
    }

    printf("Enter number of KILL events: ");
    scanf("%s", inputBuffer);

    if (!isValidInteger(inputBuffer)) {
        printf("ERROR: Invalid kill event count\n");
        return 0;
    }

    killEventCount = atoi(inputBuffer);

    for (int i = 0; i < killEventCount; i++) {
        char pidStr[16], timeStr[16];
        printf("Enter KILL <pid time>: ");
        scanf("%s %s", pidStr, timeStr);

        if (!isValidInteger(pidStr) || !isValidInteger(timeStr)) {
            printf("ERROR: Invalid KILL input\n");
            return 0;
        }

        killEvents[i].processId = atoi(pidStr);
        killEvents[i].killTime = atoi(timeStr);

        if (!findProcessById(killEvents[i].processId) || killEvents[i].killTime < 0) {
            printf("ERROR: Invalid KILL event\n");
            return 0;
        }
    }

    PCB *runningProcess = NULL;

    while (!isQueueEmpty(&readyQueue) || !isQueueEmpty(&waitingQueue) || runningProcess) {
        processKillEvents(systemClock, &readyQueue, &waitingQueue, &runningProcess);
        systemClock++;
        updateWaitingProcesses(&waitingQueue, &readyQueue);

        if (!runningProcess && !isQueueEmpty(&readyQueue)) {
            runningProcess = dequeueProcess(&readyQueue);
            runningProcess->state = STATE_RUNNING;
        }

        if (runningProcess) {
            runningProcess->executedTime++;

            if (runningProcess->ioDuration > 0 &&
                runningProcess->executedTime == runningProcess->ioStartTime) {
                runningProcess->state = STATE_WAITING;
                runningProcess->remainingIoTime = runningProcess->ioDuration;
                enqueueProcess(&waitingQueue, runningProcess);
                runningProcess = NULL;
            }
            else if (runningProcess->executedTime == runningProcess->cpuBurstTime) {
                runningProcess->state = STATE_TERMINATED;
                runningProcess->completionTime = systemClock;
                runningProcess = NULL;
            }
        }
    }

    printf("\nPID\tName\tCPU\tIO\tStatus\tTurnaround\tWaiting\n");

    for (int i = 0; i < processCount; i++) {
        PCB *process = allProcesses[i];

        if (process->state == STATE_KILLED) {
            printf("%d\t%s\t%d\t%d\tKILLED@%d\t-\t-\n",
                   process->processId, process->processName,
                   process->cpuBurstTime, process->ioDuration,
                   process->completionTime);
        } else {
            int turnaroundTime = process->completionTime;
            int waitingTime = turnaroundTime - process->cpuBurstTime - process->ioDuration;

            printf("%d\t%s\t%d\t%d\tOK\t%d\t\t%d\n",
                   process->processId, process->processName,
                   process->cpuBurstTime, process->ioDuration,
                   turnaroundTime, waitingTime);
        }
    }

    return 0;
}
