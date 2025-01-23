#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_TRAINS_ON_TRACK 3
#define TRACKS_NUMBER 4
#define SHM_KEY 12345

typedef struct {
    int train_id;
    int priority;
    int track;
} Train;

typedef struct {
    Train queue[MAX_TRAINS_ON_TRACK];
    int queue_size;
    int trains_to_dump;
    sem_t track_mutex;
} Track;

typedef struct {
    Track tracks[TRACKS_NUMBER];
    int tunnel_busy;
    sem_t memory_mutex;       
    sem_t tunnel_access; 
} SharedMemory;


