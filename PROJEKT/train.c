#include "common.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s train_id priority direction track\n", argv[0]);
        exit(1);
    }

    int train_id = atoi(argv[1]);
    int priority = atoi(argv[2]);
    int direction = atoi(argv[3]);
    int track = atoi(argv[4]);

    int shmid = shmget(12345, sizeof(SharedMemory), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed");
        exit(1);
    }

    sem_wait(&shm->mutex);

    if (shm->tracks[track].queue_size < MAX_TRAINS) {
        shm->tracks[track].queue[shm->tracks[track].queue_size].train_id = train_id;
        shm->tracks[track].queue[shm->tracks[track].queue_size].priority = priority;
        shm->tracks[track].queue[shm->tracks[track].queue_size].direction = direction;
        shm->tracks[track].queue_size++;
        printf("Train %d registered: priority=%d, direction=%d on track %c\n",
               train_id, priority, direction, 'A' + track);
    } else {
        fprintf(stderr, "Track %c queue is full, train %d cannot register.\n", 'A' + track, train_id);
    }

    sem_post(&shm->mutex);

    sem_wait(&shm->tunnel_access);
    printf("Train %d entering/exiting tunnel\n", train_id);

    sleep(2);

    sem_wait(&shm->mutex);
    shm->tunnel_busy = 0;
    sem_post(&shm->mutex);

    printf("Train %d exited tunnel\n", train_id);

    return 0;
}
