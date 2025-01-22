#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>

#define MAX_TRAINS_ON_TRACK 3
#define TRACKS_NUMBER 4

typedef struct {
    int train_id;
    int priority;
    int track;
} Train;

typedef struct {
    Train queue[MAX_TRAINS_ON_TRACK];
    int queue_size;
    sem_t track_mutex;
} Track;

typedef struct {
    Track tracks[TRACKS_NUMBER];
    int tunnel_busy;
    sem_t generator_mutex;         // Synchronizacja dostępu do pamięci
    sem_t tunnel_access; // Kontrola wjazdu do tunelu
} SharedMemory;


