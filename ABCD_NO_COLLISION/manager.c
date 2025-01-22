#include "shared.h"

int get_highest_priority_train(int* valid_trains){
    int priority = 0;
    for (int i = 0; i < TRACKS; i++) {
        if (valid_trains[i] > priority)
            priority = (valid_trains[i]);
    } 

    for (int i = 0; i < TRACKS; i++){
        if (valid_trains[i] == priority)
            return i;
    }

    return -1;
}

int get_number_of_valid_trains(int* valid_trains){
    int valid_trains_no = 0;
    for (int i = 0; i < TRACKS; i++) {
        if (valid_trains[i] > 0)
            valid_trains_no++;
    }
    return valid_trains_no;
}

bool can_train_pass(int track_id, int* valid_trains){
    if (track_id + 1 > TRACKS/2){
        for (int i = 0; i < TRACKS/2; ++i){
            if (valid_trains[i] == 0)
                return true;
        }
    }
    if (track_id + 1 < TRACKS/2){
        for (int i = TRACKS/2 ; i < TRACKS; ++i){
            if (valid_trains[i] == 0)
                return true;
        }
    }
    return false;
}

// Funkcja obsługi kolejki pociągów
void process_queue(SharedMemory *shm) {
    while (1) {
        sem_wait(&shm->mutex);  // Blokowanie dostępu do pamięci współdzielonej

        Train trains[TRACKS];
        int valid_trains[TRACKS] = {0};

        for (int i=0; i<TRACKS; i++){
            if (shm->tracks[i].queue_size > 0){
                trains[i] = shm->tracks[i].queue[0];
                valid_trains[i] = shm->tracks[i].queue[0].priority;
            }
        }

        printf("Rozpatrywane pociągi:\n");
        for (int i = 0; i < TRACKS; i++) {
            if (valid_trains[i]) {  // Sprawdzamy, które pociągi zostały zapisane
                printf("Tor %d: Pociąg ID=%d, Priorytet=%d", 
                    i, 
                    trains[i].id, 
                    trains[i].priority
                );
            }
        }


        int track_to_move = -1;
        int max_it = TRACKS;
        while(get_number_of_valid_trains(valid_trains) > 0){
            int track_id = get_highest_priority_train(valid_trains);
            if(can_train_pass(track_id, valid_trains)){
                track_to_move = track_id;
                break;
            }
            max_it--;
            if (max_it == 0)
                break;
            valid_trains[track_id] = 0;
        }

        if (track_to_move == -1) {
            int tr = rand() % TRACKS;
            printf("Collision detected. Removing trains from track %d\n", tr);
            sleep(1);
            shm->tracks[tr].queue_size = 0;
            sem_post(&shm->tunnel_access);
            continue;
        }

        sem_wait(&shm->tunnel_access);

        printf("Wjazd do tunelu - Tor %d: Pociąg ID=%d, Priorytet=%d", 
                    track_to_move, 
                    trains[track_to_move].id, 
                    trains[track_to_move].priority
                );
        
        for (int i = 0; i < shm->tracks[track_to_move].queue_size - 1; i++) {
                    shm->tracks[track_to_move].queue[i] = shm->tracks[track_to_move].queue[i + 1];
                }
                shm->tracks[track_to_move].queue_size--;

        sem_post(&shm->tunnel_access);





        // if (shm.queue_size > 0 && shm->tunnel_busy == 0) {
        //     Train train = shm.queue[0];

        //     // Sprawdzenie, czy pociąg może wjechać do tunelu
        //     if ((train.source == 'A' || train.source == 'B') && 
        //         (train.destination == 'C' || train.destination == 'D')) {

        //         printf("Train %d entering tunnel from %c to %c (priority %d)\n",
        //             train.id, train.source, train.destination, train.priority);

        //         shm->tunnel_busy = 1;

        //         // Usuwanie pierwszego pociągu z kolejki
        //         for (int i = 0; i < shm.queue_size - 1; i++) {
        //             shm.queue[i] = shm.queue[i + 1];
        //         }
        //         shm.queue_size--;

        //         sem_post(&shm->tunnel_access);
        //     }
        //     else if ((train.source == 'C' || train.source == 'D') && 
        //         (train.destination == 'A' || train.destination == 'B')) {

        //         printf("Train %d entering tunnel from %c to %c (priority %d)\n",
        //             train.id, train.source, train.destination, train.priority);

        //         shm->tunnel_busy = 1;

        //         // Usuwanie pierwszego pociągu z kolejki
        //         for (int i = 0; i < shm.queue_size - 1; i++) {
        //             shm.queue[i] = shm.queue[i + 1];
        //         }
        //         shm.queue_size--;

        //         sem_post(&shm->tunnel_access);
        //     }
        // }
        
        sem_post(&shm->mutex);
        sleep(3); // Kontrola cyklu zarządzania
    }
}

int main() {
    // Tworzenie pamięci współdzielonej
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
    for (int i = 0; i < TRACKS; i++) {
        shm->tracks[i].queue_size = 0;
    }

    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->tunnel_access, 1, 0);

    printf("Tunnel manager started.\n");

    process_queue(shm);

    return 0;
}
