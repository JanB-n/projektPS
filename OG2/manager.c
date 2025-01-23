#include "common.h"

int shmid;
SharedMemory* shm;

void cleanup(int signum) {
    printf("\nCaught signal %d, cleaning up shared memory...\n", signum);
    
    // Usuwanie semaforów
    for (int i = 0; i < TRACKS_NUMBER; i++) {
        sem_destroy(&shm->tracks[i].track_mutex);
    }
    sem_destroy(&shm->memory_mutex);
    sem_destroy(&shm->tunnel_access);
    
    // Odłączenie pamięci współdzielonej
    if (shmdt(shm) == -1) {
        perror("shmdt failed");
    }
    
    // Usunięcie segmentu pamięci współdzielonej
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    } else {
        printf("Shared memory removed successfully.\n");
    }

    exit(0);
}

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
    if (track_id < TRACKS_NUMBER / 2) {
        for (int i = TRACKS_NUMBER / 2; i < TRACKS_NUMBER; ++i) {
            if (valid_trains[i] == 0)
                return true;
        }
    }
    return false;
}

void process_queue(SharedMemory* shm) {
    while (1) {
        sem_wait(&shm->memory_mutex); // Blokowanie dostępu do pamięci współdzielonej
        // if(shm->tunnel_busy == 0){
            int valid_trains[TRACKS_NUMBER] = {0};

            // Sprawdzamy pociągi w kolejkach
            int sum = 0;
            for (int i = 0; i < TRACKS_NUMBER; i++) {
                if (shm->tracks[i].queue_size > 0) {
                    valid_trains[i] = shm->tracks[i].queue[0].priority;
                    sum += valid_trains[i];
                }
            }
            if(sum == 0){
                printf("No trains detected\n");
                sem_post(&shm->memory_mutex);
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
            int current_trains[TRACKS_NUMBER] = {0}; 
            for (int i = 0; i < TRACKS_NUMBER; i++){
                current_trains[i] = valid_trains[i];
            }
            while (get_number_of_valid_trains(current_trains) > 0) {
                int track_id = get_highest_priority_train(current_trains);
                if (can_train_pass(track_id, valid_trains)) {
                    track_to_move = track_id;
                    break;
                }
                max_it--;
                if (max_it == 0)
                    break;
                current_trains[track_id] = 0;
            }

            if (track_to_move == -1) {
                int tr = rand() % TRACKS_NUMBER;
                printf("Collision detected. Removing trains from track %d\n", tr);
                sleep(1);
                shm->tracks[tr].trains_to_dump = shm->tracks[tr].queue_size;
                for(int i=0; i<shm->tracks[tr].queue_size; ++i){
                    sem_post(&shm->tracks[tr].track_mutex);
                }
                shm->tracks[tr].queue_size = 0;

                sem_post(&shm->memory_mutex);
                continue;
            }

            sem_post(&shm->tunnel_access); // Synchronizacja dostępu do tunelu
            sem_post(&shm->tracks[track_to_move].track_mutex);
            printf("Wjazd do tunelu - Tor %d: Pociąg ID=%d, Priorytet=%d\n", 
                track_to_move, shm->tracks[track_to_move].queue[0].train_id, shm->tracks[track_to_move].queue[0].priority);
            // shm->tunnel_busy = 1;
            // Przemieszczanie pociągu z kolejki
            for (int i = 0; i < shm->tracks[track_to_move].queue_size - 1; i++) {
                shm->tracks[track_to_move].queue[i] = shm->tracks[track_to_move].queue[i + 1];
            }
            shm->tracks[track_to_move].queue_size--;
        //}
        sem_post(&shm->memory_mutex); // Zwolnienie semafora
        sleep(3); // Kontrola cyklu
    }
}

int main() {
    // Uzyskanie dostępu do pamięci współdzielonej
    // int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), 0666);
    signal(SIGINT, cleanup);
    shmid = shmget(12345, sizeof(SharedMemory), IPC_CREAT | 0666);
    printf("Shared memory created with key 12345. Shmid: %d\n", shmid);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed");
        exit(1);
    }

    for (int i = 0; i < TRACKS_NUMBER; i++) {
        shm->tracks[i].trains_to_dump = 0;
        shm->tracks[i].queue_size = 0;
        sem_init(&shm->tracks[i].track_mutex, 1, 0);
    }
    shm->tunnel_busy = 0;
    sem_init(&shm->memory_mutex, 1, 1);         // Semafor mutex
    sem_init(&shm->tunnel_access, 1, 0); // Semafor tunelu


    printf("Tunnel manager started.\n");

    // Uruchomienie obsługi kolejki pociągów
    process_queue(shm);

    return 0;
}
