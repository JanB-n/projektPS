#include "common.h"

int get_highest_priority_train(int* valid_trains) {
    int priority = 0;
    for (int i = 0; i < TRACKS_NUMBER; i++) {
        if (valid_trains[i] > priority)
            priority = valid_trains[i];
    }

    for (int i = 0; i < TRACKS_NUMBER; i++) {
        if (valid_trains[i] == priority)
            return i;
    }

    return -1;
}

int get_number_of_valid_trains(int* valid_trains) {
    int valid_trains_no = 0;
    for (int i = 0; i < TRACKS_NUMBER; i++) {
        if (valid_trains[i] > 0)
            valid_trains_no++;
    }
    return valid_trains_no;
}

bool can_train_pass(int track_id, int* valid_trains) {
    if (track_id + 1 > TRACKS_NUMBER / 2) {
        for (int i = 0; i < TRACKS_NUMBER / 2; ++i) {
            if (valid_trains[i] == 0)
                return true;
        }
    }
    if (track_id + 1 < TRACKS_NUMBER / 2) {
        for (int i = TRACKS_NUMBER / 2; i < TRACKS_NUMBER; ++i) {
            if (valid_trains[i] == 0)
                return true;
        }
    }
    return false;
}

void process_queue(SharedMemory* shm) {
    while (1) {
        sem_wait(&shm->generator_mutex); // Blokowanie dostępu do pamięci współdzielonej
        int valid_trains[TRACKS_NUMBER] = {0};

        // Sprawdzamy pociągi w kolejkach
        int sum = 0;
        for (int i = 0; i < TRACKS_NUMBER; i++) {
            printf("track: %d, queue size:: %d\n", i, shm->tracks[i].queue_size);
            if (shm->tracks[i].queue_size > 0) {
                valid_trains[i] = shm->tracks[i].queue[0].priority;
                sum += valid_trains[i];
            }
        }
        if(sum == 0){
            printf("No trains detected\n");
            sem_post(&shm->generator_mutex);
            sleep(5);
            continue;
        }

        printf("Rozpatrywane pociągi:\n");
        for (int i = 0; i < TRACKS_NUMBER; i++) {
            if (valid_trains[i]) {
                printf("Tor %d: Pociąg ID=%d, Priorytet=%d\n", i, shm->tracks[i].queue[0].train_id, shm->tracks[i].queue[0].priority);
            }
        }

        int track_to_move = -1;
        int max_it = TRACKS_NUMBER;
        while (get_number_of_valid_trains(valid_trains) > 0) {
            int track_id = get_highest_priority_train(valid_trains);
            if (can_train_pass(track_id, valid_trains)) {
                track_to_move = track_id;
                break;
            }
            max_it--;
            if (max_it == 0)
                break;
            valid_trains[track_id] = 0;
        }

        if (track_to_move == -1) {
            int tr = rand() % TRACKS_NUMBER;
            printf("Collision detected. Removing trains from track %d\n", tr);
            sleep(1);
            shm->tracks[tr].queue_size = 0;
            sem_post(&shm->generator_mutex);
            continue;
        }

        sem_post(&shm->tunnel_access); // Synchronizacja dostępu do tunelu
        sem_post(&shm->tracks[track_to_move].track_mutex);
        printf("Wjazd do tunelu - Tor %d: Pociąg ID=%d, Priorytet=%d\n", 
               track_to_move, shm->tracks[track_to_move].queue[0].train_id, shm->tracks[track_to_move].queue[0].priority);

        // Przemieszczanie pociągu z kolejki
        for (int i = 0; i < shm->tracks[track_to_move].queue_size - 1; i++) {
            shm->tracks[track_to_move].queue[i] = shm->tracks[track_to_move].queue[i + 1];
        }
        shm->tracks[track_to_move].queue_size--;

        sem_post(&shm->generator_mutex); // Zwolnienie semafora
        sleep(3); // Kontrola cyklu
    }

    // while (1) {
    //     sem_wait(&shm->generator_mutex); // Zablokowanie dostępu do pamięci współdzielonej

    //     if (shm->queue_size > 0 && shm->tunnel_busy == 0) {
    //         // Wybierz pociąg o najwyższym priorytecie
    //         int highest_priority = -1, train_index = -1;
    //         for (int i = 0; i < shm->queue_size; i++) {
    //             if (shm->queue[i].priority > highest_priority) {
    //                 highest_priority = shm->queue[i].priority;
    //                 train_index = i;
    //             }
    //         }

    //         // Zezwolenie na wjazd pociągu
    //         if (train_index != -1) {
    //             printf("Allowing train %d (priority=%d, direction=%d) to enter the tunnel\n",
    //                    shm->queue[train_index].train_id,
    //                    shm->queue[train_index].priority,
    //                    shm->queue[train_index].direction);

    //             shm->tunnel_busy = 1;

    //             // Usuń pociąg z kolejki
    //             for (int i = train_index; i < shm->queue_size - 1; i++) {
    //                 shm->queue[i] = shm->queue[i + 1];
    //             }
    //             shm->queue_size--;

    //             // Powiadomienie pociągu
    //             sem_post(&shm->tunnel_access);
    //         }
    //     }

    //     sem_post(&shm->generator_mutex); // Odblokowanie dostępu do pamięci współdzielonej
    //     usleep(100000); // Krótkie opóźnienie dla symulacji pracy zarządcy
    // }
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

    for (int i = 0; i < TRACKS_NUMBER; i++) {
        shm->tracks[i].queue_size = 0;
        sem_init(&shm->tracks[i].track_mutex, 1, 0);
    }
    shm->tunnel_busy = 0;
    sem_init(&shm->generator_mutex, 1, 1);         // Semafor mutex
    sem_init(&shm->tunnel_access, 1, 1); // Semafor tunelu


    printf("Tunnel manager started.\n");

    // Uruchomienie obsługi kolejki pociągów
    process_queue(shm);

    return 0;
}
