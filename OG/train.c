#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define MAX_TRAINS 10

typedef struct {
    int train_id;
    int priority;
    int direction; // 0 - wjazd do tunelu, 1 - wyjazd z tunelu
} Train;

typedef struct {
    Train queue[MAX_TRAINS];
    int queue_size;
    int tunnel_busy;
    sem_t mutex;         // Synchronizacja dostępu do pamięci
    sem_t tunnel_access; // Kontrola wjazdu do tunelu
} SharedMemory;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s train_id priority direction\n", argv[0]);
        exit(1);
    }

    int train_id = atoi(argv[1]);
    int priority = atoi(argv[2]);
    int direction = atoi(argv[3]);

    // Uzyskanie dostępu do pamięci współdzielonej
    // int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0666);
    int shmid = shmget(12345, sizeof(SharedMemory), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed 2");
        exit(1);
    }

    // Rejestracja pociągu w pamięci współdzielonej
    sem_wait(&shm->mutex); // Zablokowanie dostępu do pamięci współdzielonej

    if (shm->queue_size < MAX_TRAINS) {
        shm->queue[shm->queue_size].train_id = train_id;
        shm->queue[shm->queue_size].priority = priority;
        shm->queue[shm->queue_size].direction = direction;
        shm->queue_size++;
        printf("Train %d registered: priority=%d, direction=%d\n", train_id, priority, direction);
    } else {
        fprintf(stderr, "Train queue is full, train %d cannot register.\n", train_id);
    }

    sem_post(&shm->mutex); // Odblokowanie dostępu do pamięci współdzielonej

    // Oczekiwanie na pozwolenie wjazdu/wyjazdu
    sem_wait(&shm->tunnel_access);
    printf("Train %d entering/exiting tunnel\n", train_id);

    // Symulacja przejazdu przez tunel
    sleep(2);

    // Zwolnienie tunelu
    sem_wait(&shm->mutex);
    shm->tunnel_busy = 0;
    sem_post(&shm->mutex);

    printf("Train %d exited tunnel\n", train_id);

    return 0;
}
