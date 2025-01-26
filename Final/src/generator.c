/**
 * @file generator.c
 * @brief Program generujący pociągi i dodający je do pamięci współdzielonej.
 */

#include "common.h"

pid_t train_pids[MAX_TRAINS_ON_TRACK * TRACKS_NUMBER]; /**< Tablica PID-ów procesów pociągów */
int train_count = 0; /**< Licznik wygenerowanych pociągów */
int shmid; /**< Identyfikator pamięci współdzielonej */
SharedMemory* shm; /**< Wskaźnik do pamięci współdzielonej */
int is_generator = 1; /**< Flaga określająca, czy proces jest generatorem */

/**
 * @brief Funkcja obsługi sygnału SIGINT, zatrzymująca procesy.
 */
void cleanup() {
    if(is_generator){
        printf("\nCleaning generator...\n");
        // for(int i=0; i < TRACKS_NUMBER; ++i){
        //     shm->tracks[i].queue_size = 0;
        // }
        for (int i = 0; i < train_count; i++) {
            printf("Stopping train process %d...\n", train_pids[i]);
            kill(train_pids[i], SIGTERM);
        }

        printf("Finished cleaning generator.\n");
        exit(0);
    }
}

/**
 * @brief Proces reprezentujący pociąg.
 * 
 * @param train_id  ID pociągu.
 * @param priority  Priorytet pociągu.
 * @param track     Tor, na którym pociąg się znajduje.
 */
void train_process(int train_id, int priority, int track) {
    is_generator = 0;
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    SharedMemory* shm = (SharedMemory*)shmat(shmid, NULL, 0);
    if (shm == (SharedMemory*)-1) {
        perror("shmat failed");
        exit(1);
    }

    sem_wait(&shm->memory_mutex);

    if (shm->tracks[track].queue_size < MAX_TRAINS_ON_TRACK) {
        shm->tracks[track].queue[shm->tracks[track].queue_size].train_id = train_id;
        shm->tracks[track].queue[shm->tracks[track].queue_size].priority = priority;
        shm->tracks[track].queue[shm->tracks[track].queue_size].track = track;
        shm->tracks[track].queue_size++;
        printf("Train ID=%d registered: priority=%d, track=%d\n", train_id, priority, track);
    } else {
        fprintf(stderr, "Train queue is full, train ID=%d cannot register.\n", train_id);
        sem_post(&shm->memory_mutex);
        exit(1);
    }

    sem_post(&shm->memory_mutex);

    sem_wait(&shm->tracks[track].track_mutex);

    if (shm->tracks[track].trains_to_dump > 0) {
        printf("Train ID=%d was removed\n", train_id);
        shm->tracks[track].trains_to_dump--;
        exit(0);
    }

    sem_wait(&shm->tunnel_access);

    printf("Train ID=%d entering tunnel\n", train_id);
    sleep(2);
    printf("Train ID=%d leaving tunnel\n", train_id);

    exit(0);
}

/**
 * @brief Tworzy proces dla nowego pociągu.
 * 
 * @param train_id  ID pociągu.
 * @param priority  Priorytet pociągu.
 * @param track     Numer toru, na którym się znajduje.
 */
void create_train_process(int train_id, int priority, int track) {
    pid_t pid = fork();
    if (pid == 0) { 
        train_process(train_id, priority, track);
    } else if (pid > 0) {
        if (train_count < MAX_TRAINS_ON_TRACK * TRACKS_NUMBER) {
            train_pids[train_count++] = pid;
        }
    } else {
        perror("fork failed");
    }
}

/**
 * @brief Główna funkcja generatora pociągów.
 * 
 * Tworzy nowe pociągi w losowych odstępach czasu i przekazuje je do pamięci współdzielonej.
 * 
 * @return int Kod zakończenia programu.
 */
int main() {
    signal(SIGINT, cleanup);
    srand(time(NULL));

    shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
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
    sem_init(&shm->memory_mutex, 1, 1);
    sem_init(&shm->tunnel_access, 1, 0);

    printf("Tunnel manager started. Press Ctrl+C to exit.\n");

    int train_id = 0;
    while (1) {
        int priority = rand() % 3 + 1;  
        int track = rand() % TRACKS_NUMBER;  

        printf("Generating train ID=%d, priority=%d, track=%d\n", train_id, priority, track);
        create_train_process(train_id, priority, track);
        train_id++;

        sleep(rand() % 3 + 1); 
    }

    return 0;
}
