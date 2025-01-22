#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <semaphore.h>

#define MAX_TRAINS 10
#define NUM_TRACKS 4 // A, B, C, D

typedef struct {
    int train_id;
    int priority;
    int direction; // 0 - entering tunnel, 1 - exiting tunnel
} Train;

typedef struct {
    Train queue[MAX_TRAINS];
    int queue_size;
} TrackQueue;

typedef struct {
    TrackQueue tracks[NUM_TRACKS]; // One queue per track
    int tunnel_busy;
    sem_t mutex;         // Synchronization access to shared memory
    sem_t tunnel_access; // Tunnel entry control
} SharedMemory;

#endif // COMMON_H
