#include "common.h"

void process_queue(SharedMemory* shm) {
    while (1) {
        sem_wait(&shm->mutex);

        if (shm->tunnel_busy == 0) {
            int selected_track = -1, highest_priority = -1, train_index = -1;

            // Find the highest priority train across all tracks
            for (int t = 0; t < NUM_TRACKS; t++) {
                for (int i = 0; i < shm->tracks[t].queue_size; i++) {
                    if (shm->tracks[t].queue[i].priority > highest_priority) {
                        highest_priority = shm->tracks[t].queue[i].priority;
                        selected_track = t;
                        train_index = i;
                    }
                }
            }

            if (train_index != -1) {
                Train train = shm->tracks[selected_track].queue[train_index];
                printf("Allowing train %d (priority=%d, direction=%d) from track %c to enter the tunnel\n",
                       train.train_id, train.priority, train.direction, 'A' + selected_track);

                shm->tunnel_busy = 1;

                // Remove the train from the selected track queue
                for (int i = train_index; i < shm->tracks[selected_track].queue_size - 1; i++) {
                    shm->tracks[selected_track].queue[i] = shm->tracks[selected_track].queue[i + 1];
                }
                shm->tracks[selected_track].queue_size--;

                // Notify the train
                sem_post(&shm->tunnel_access);
            }
        }

        sem_post(&shm->mutex);
        usleep(100000);
    }
}

int main() {
    int shmid = shmget(12345, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed");
        exit(1);
    }

    for (int i = 0; i < NUM_TRACKS; i++) {
        shm->tracks[i].queue_size = 0;
    }
    shm->tunnel_busy = 0;
    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->tunnel_access, 1, 0);

    printf("Tunnel manager started.\n");

    process_queue(shm);

    return 0;
}
