#include "shared.h"

void create_train_process(int id, int track, int priority, SharedMemory *shm) {
    sem_wait(&shm->mutex); // Blokowanie dostępu do pamięci współdzielonej

    // Czekamy, aż będzie miejsce w kolejce
    while (shm->tracks[track].queue_size >= MAX_TRAINS_ON_1_TRACK) {
        printf("Train queue is full! Waiting to create train %d on track %d...\n", id, track);
        sem_post(&shm->mutex); // Zwolnienie semafora przed snem
        sleep(1); // Czekamy 1 sekundę przed ponowną próbą
        sem_wait(&shm->mutex); // Ponowne zablokowanie semafora
    }

    // Dodanie pociągu do kolejki
    Train new_train = {id, track, priority};
    shm->tracks[track].queue[shm->tracks[track].queue_size++] = new_train;
    printf("Generated train ID=%d from track %d (priority %d)\n", id, track, priority);

    sem_post(&shm->mutex); // Zwolnienie semafora
}

int main() {
    srand(time(NULL));

    int shmid = shmget(12345, sizeof(SharedMemory), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    SharedMemory *shm = (SharedMemory *)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Inicjalizacja pamięci współdzielonej
    for (int i = 0; i < TRACKS; i++) {
        shm->tracks[i].queue_size = 0;
    }

    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->tunnel_access, 1, 0);

    int train_id = 0;
    while (1) {
        int priority = rand() % 3 + 1; // Priorytety 1-3
        int track = rand() % TRACKS;

        create_train_process(train_id++, track, priority, shm);
        sleep(rand() % 5 + 1);
    }

    return 0;
}
