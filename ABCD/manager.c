#include "common.h"

void process_queue(SharedMemory *shm) {
    while (1) {
        sem_wait(&shm->mutex);

        if (shm->queue_size > 0 && !shm->tunnel_busy) {
            Train next_train = shm->queue[0];

            // Sprawdzanie poprawności kierunku (A/B -> C/D lub C/D -> A/B)
            if ((next_train.entry_line <= 1 && next_train.exit_line >= 2) ||
                (next_train.entry_line >= 2 && next_train.exit_line <= 1)) {

                printf("Train ID=%d entering tunnel from %c to %c\n",
                       next_train.train_id,
                       (next_train.entry_line == 0 ? 'A' : next_train.entry_line == 1 ? 'B' : next_train.entry_line == 2 ? 'C' : 'D'),
                       (next_train.exit_line == 0 ? 'A' : next_train.exit_line == 1 ? 'B' : next_train.exit_line == 2 ? 'C' : 'D'));

                shm->tunnel_busy = 1;
                sem_post(&shm->tunnel_access);
                sleep(2);  // Symulacja przejazdu przez tunel
                
                printf("Train ID=%d exited tunnel\n", next_train.train_id);
                shm->tunnel_busy = 0;

                // Usunięcie pociągu z kolejki
                for (int i = 0; i < shm->queue_size - 1; i++) {
                    shm->queue[i] = shm->queue[i + 1];
                }
                shm->queue_size--;
            } else {
                printf("Invalid train direction! Train ID=%d blocked.\n", next_train.train_id);
            }
        }

        sem_post(&shm->mutex);
        sleep(1);
    }
}

int main() {
    int shmid = shmget(12345, sizeof(SharedMemory), IPC_CREAT | 0666);
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
    shm->queue_size = 0;
    shm->tunnel_busy = 0;
    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->tunnel_access, 1, 0);

    printf("Tunnel manager started.\n");
    process_queue(shm);

    return 0;
}
