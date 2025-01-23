#include "common.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s train_id priority track\n", argv[0]);
        exit(1);
    }

    int train_id = atoi(argv[1]);
    int priority = atoi(argv[2]);
    int track = atoi(argv[3]);

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
    sem_wait(&shm->memory_mutex); // Zablokowanie dostępu do pamięci współdzielonej

    if (shm->tracks[track].queue_size < MAX_TRAINS_ON_TRACK) {
        shm->tracks[track].queue[shm->tracks[track].queue_size].train_id = train_id;
        shm->tracks[track].queue[shm->tracks[track].queue_size].priority = priority;
        shm->tracks[track].queue[shm->tracks[track].queue_size].track = track;
        shm->tracks[track].queue_size++;
        printf("Train %d registered: priority=%d, track=%d\n", train_id, priority, track);
    } else {
        fprintf(stderr, "Train queue is full, train %d cannot register.\n", train_id);
    }

    sem_post(&shm->memory_mutex); // Odblokowanie dostępu do pamięci współdzielonej

    // Oczekiwanie na pozwolenie wjazdu/wyjazdu
    
    sem_wait(&shm->tracks[track].track_mutex);

    if(shm->tracks[track].trains_to_dump > 0){
        printf("Train %d was removed\n", train_id);
        shm->tracks[track].trains_to_dump--;
        // shm->tunnel_busy = 0;
        return 0;
    }
    sem_wait(&shm->tunnel_access);

    //sem_wait(&shm->memory_mutex);
    
    //sem_post(&shm->memory_mutex);
    printf("Train %d entering tunnel\n", train_id);

    // Symulacja przejazdu przez tunel
    sleep(2);

    // Zwolnienie tunelu
    sem_wait(&shm->memory_mutex);
    // shm->tunnel_busy = 0;
    sem_post(&shm->memory_mutex);

    printf("Train %d exited tunnel\n", train_id);

    return 0;
}
