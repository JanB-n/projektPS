#include "common.h"

void create_train_process(int train_id, int priority, int entry_line, int exit_line, SharedMemory *shm) {
    sem_wait(&shm->mutex);

    if (shm->queue_size < MAX_TRAINS) {
        Train new_train = {train_id, priority, entry_line, exit_line};
        shm->queue[shm->queue_size++] = new_train;
        printf("Train ID=%d added to queue (priority=%d, from %c to %c)\n",
               train_id, priority,
               (entry_line == 0 ? 'A' : entry_line == 1 ? 'B' : entry_line == 2 ? 'C' : 'D'),
               (exit_line == 0 ? 'A' : exit_line == 1 ? 'B' : exit_line == 2 ? 'C' : 'D'));
    } else {
        printf("Train queue is full! Train ID=%d discarded.\n", train_id);
    }

    sem_post(&shm->mutex);
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

    int train_id = 0;
    while (1) {
        int entry = rand() % 2;       // A (0) lub B (1)
        int exit = (rand() % 2) + 2;   // C (2) lub D (3)

        if (rand() % 2) {  // Losowy wybór strony tunelu
            entry += 2;  // C (2) lub D (3)
            exit -= 2;   // A (0) lub B (1)
        }

        int priority = rand() % 3 + 1;
        printf("Generating train ID=%d, priority=%d, entry=%c, exit=%c\n",
               train_id, priority,
               (entry == 0 ? 'A' : entry == 1 ? 'B' : entry == 2 ? 'C' : 'D'),
               (exit == 0 ? 'A' : exit == 1 ? 'B' : exit == 2 ? 'C' : 'D'));

        create_train_process(train_id, priority, entry, exit, shm);
        train_id++;

        sleep(rand() % 3 + 1);
    }

    return 0;
}
