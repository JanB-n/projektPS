#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>

#define MAX_TRAINS 5

typedef struct {
    int train_id;
    int priority;   // 1 - ekspres, 2 - pospieszny, 3 - towarowy
    int entry_line; // 0 = A, 1 = B, 2 = C, 3 = D
    int exit_line;  // 0 = A, 1 = B, 2 = C, 3 = D
} Train;

typedef struct {
    Train queue[MAX_TRAINS];
    int queue_size;
    int tunnel_busy;
    sem_t mutex;
    sem_t tunnel_access;
} SharedMemory;

void create_train_process(int train_id, int priority, int entry_line, int exit_line, SharedMemory *shm);
void process_queue(SharedMemory *shm);

#endif
