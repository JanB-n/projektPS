#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

// Maksymalna liczba pociągów w kolejce
#define MAX_TRAINS_ON_1_TRACK 3
#define TRACKS 4

// Struktura pociągu
typedef struct {
    int id;
    int source;
    int priority; // 1 - ekspres, 2 - pospieszny, 3 - towarowy
} Train;

typedef struct {
    Train queue[MAX_TRAINS_ON_1_TRACK];
    int queue_size;
    
} Track;

// Struktura pamięci współdzielonej
typedef struct {
    Track tracks[TRACKS];
    sem_t mutex;        
    sem_t tunnel_access;  // Kontrola wjazdu do tunelu
} SharedMemory;

// bool isOppositeTrack(int track_number){

// }

#endif
