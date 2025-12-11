#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    READY,
    RUNNING,
    WAITING,
    TERMINATED
} ProcessState;

typedef struct Process {
    char name[50];
    int pid;
    int cpuBurst;
    int ioStartTime;
    int ioDuration;

    int executedCpuTime;
    int executedIoTime;
    int completionTime;

    ProcessState state;
    struct Process *next;
} Process;

typedef struct Queue {
    Process *front;
    Process *rear;
} Queue;

typedef struct {
    int pid;
    int killTime;
} KillEvent;

void initializeQueue(Queue *queue) {
    queue->front = queue->rear = NULL;
}

void enqueue(Queue *queue, Process *process) {
    process->next = NULL;
    if (!queue->rear) {
        queue->front = queue->rear = process;
    } else {
        queue->rear->next = process;
        queue->rear = process;
    }
}

Process* dequeue(Queue *queue) {
    if (!queue->front) return NULL;
    Process *p = queue->front;
    queue->front = queue->front->next;
    if (!queue->front) queue->rear = NULL;
    p->next = NULL;
    return p;
}

int isQueueEmpty(Queue *queue) {
    return queue->front == NULL;
}

int main() {
    int numberOfProcesses;
    printf("Enter number of processes:\n");
    scanf("%d", &numberOfProcesses);

    Queue readyQueue, waitingQueue, terminatedQueue;
    initializeQueue(&readyQueue);
    initializeQueue(&waitingQueue);
    initializeQueue(&terminatedQueue);

    Process *processList[numberOfProcesses];

    printf("\nEnter: name pid cpuBurst ioStart ioDuration\n");
    for (int i = 0; i < numberOfProcesses; i++) {
        Process *p = malloc(sizeof(Process));
        scanf("%s %d %d %d %d",
              p->name, &p->pid, &p->cpuBurst,
              &p->ioStartTime, &p->ioDuration);

        p->executedCpuTime = 0;
        p->executedIoTime = -1; 
        p->completionTime = 0;
        p->state = READY;
        p->next = NULL;

        processList[i] = p;
        enqueue(&readyQueue, p);
    }

    int numberOfKillEvents;
    printf("\nEnter number of kill events:\n");
    scanf("%d", &numberOfKillEvents);

    KillEvent killEvents[numberOfKillEvents];
    if (numberOfKillEvents > 0)
        printf("Enter kill events: pid killTime\n");
    for (int i = 0; i < numberOfKillEvents; i++)
        scanf("%d %d", &killEvents[i].pid, &killEvents[i].killTime);

    Process *runningProcess = NULL;
    int currentTime = 0;

    printf("\n--------------- Execution Log ---------------\n");

    while (1) {

        for (int idx = 0; idx < numberOfKillEvents; idx++) {
            if (killEvents[idx].killTime == currentTime) {
                for (int val = 0; val < numberOfProcesses; val++) {
                    Process *p = processList[val];
                    if (p->pid == killEvents[idx].pid &&
                        p->state != TERMINATED) {

                        p->state = TERMINATED;
                        p->completionTime = currentTime;
                        enqueue(&terminatedQueue, p);

                        printf("[Time=%d] Process %s (%d) terminated by kill\n",
                               currentTime, p->name, p->pid);

                        if (runningProcess == p)
                            runningProcess = NULL;
                    }
                }
            }
        }

        if (runningProcess && runningProcess->state == RUNNING) {
            runningProcess->executedCpuTime++;

            if (runningProcess->executedCpuTime == runningProcess->ioStartTime
                && runningProcess->ioDuration > 0) {

                runningProcess->state = WAITING;
                runningProcess->executedIoTime = -1; 

                enqueue(&waitingQueue, runningProcess);

                printf("[Time=%d] Process %s moved to I/O\n",
                       currentTime, runningProcess->name);

                runningProcess = NULL;
            }
            else if (runningProcess->executedCpuTime >= runningProcess->cpuBurst) {
                runningProcess->state = TERMINATED;
                runningProcess->completionTime = currentTime;
                enqueue(&terminatedQueue, runningProcess);

                printf("[Time=%d] Process %s completed execution\n",
                       currentTime, runningProcess->name);

                runningProcess = NULL;
            }
        }

        Process *prev = NULL, *cur = waitingQueue.front;

        while (cur) {
            cur->executedIoTime++;

            if (cur->executedIoTime >= cur->ioDuration) {
                cur->state = READY;

                printf("[Time=%d] I/O completed for: %s\n",
                       currentTime, cur->name);

                Process *done = cur;
                if (!prev) waitingQueue.front = cur->next;
                else prev->next = cur->next;

                if (cur == waitingQueue.rear)
                    waitingQueue.rear = prev;

                enqueue(&readyQueue, done);

                cur = (!prev) ? waitingQueue.front : prev->next;
                continue;
            }

            prev = cur;
            cur = cur->next;
        }

        if (!runningProcess) {
            runningProcess = dequeue(&readyQueue);

            if (runningProcess && runningProcess->state != TERMINATED) {
                runningProcess->state = RUNNING;
                printf("[Time=%d] Running: %s (%d)\n",
                       currentTime, runningProcess->name, runningProcess->pid);
            }
        }

        currentTime++;

        if (!runningProcess &&
            isQueueEmpty(&readyQueue) &&
            isQueueEmpty(&waitingQueue))
            break;
    }

    printf("\n-------- Final Scheduling Result --------\n");
    printf("PID     Name    CPU     IO      TAT     Waiting\n");

    Process *p = terminatedQueue.front;
    while (p) {
        int tat = p->completionTime;
        int waiting = tat - p->cpuBurst - p->ioDuration;
        if (waiting < 0) waiting = 0;

        printf("%d\t%s\t%d\t%d\t%d\t%d\n",
               p->pid, p->name,
               p->cpuBurst,
               p->ioDuration,
               tat,
               waiting);

        p = p->next;
    }
    printf("-----------------------------------------\n");

    return 0;
}
