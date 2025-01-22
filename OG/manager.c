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

void process_queue(SharedMemory* shm) {
    sleep(20);
    printf("hi!\n");
    while (1) {
        sem_wait(&shm->mutex); // Zablokowanie dostępu do pamięci współdzielonej

        if (shm->queue_size > 0 && shm->tunnel_busy == 0) {
            // Wybierz pociąg o najwyższym priorytecie
            int highest_priority = -1, train_index = -1;
            for (int i = 0; i < shm->queue_size; i++) {
                if (shm->queue[i].priority > highest_priority) {
                    highest_priority = shm->queue[i].priority;
                    train_index = i;
                }
            }

            // Zezwolenie na wjazd pociągu
            if (train_index != -1) {
                printf("Allowing train %d (priority=%d, direction=%d) to enter the tunnel\n",
                       shm->queue[train_index].train_id,
                       shm->queue[train_index].priority,
                       shm->queue[train_index].direction);

                shm->tunnel_busy = 1;

                // Usuń pociąg z kolejki
                for (int i = train_index; i < shm->queue_size - 1; i++) {
                    shm->queue[i] = shm->queue[i + 1];
                }
                shm->queue_size--;

                // Powiadomienie pociągu
                sem_post(&shm->tunnel_access);
            }
        }

        sem_post(&shm->mutex); // Odblokowanie dostępu do pamięci współdzielonej
        usleep(100000); // Krótkie opóźnienie dla symulacji pracy zarządcy
    }
}

int main() {
    // Uzyskanie dostępu do pamięci współdzielonej
    // int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0666);
    int shmid = shmget(12345, sizeof(SharedMemory), IPC_CREAT | 0666);
    printf("Shared memory created with key 12345. Shmid: %d\n", shmid);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed");
        exit(1);
    }

    shm->queue_size = 0;
    shm->tunnel_busy = 0;
    sem_init(&shm->mutex, 1, 1);         // Semafor mutex
    sem_init(&shm->tunnel_access, 1, 0); // Semafor tunelu


    printf("Tunnel manager started.\n");

    // Uruchomienie obsługi kolejki pociągów
    process_queue(shm);

    return 0;
}
