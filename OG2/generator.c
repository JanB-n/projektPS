#include "common.h"

void create_train_process(int train_id, int priority, int direction, SharedMemory* shm) {
    pid_t pid = fork();
    if (pid == 0) { // Proces dziecka - pociąg
        char train_id_str[10], priority_str[10], direction_str[10];
        sprintf(train_id_str, "%d", train_id);
        sprintf(priority_str, "%d", priority);
        sprintf(direction_str, "%d", direction);

        execlp("./train", "./train", train_id_str, priority_str, direction_str, NULL);
        perror("execlp failed"); // Jeśli nie uda się uruchomić procesu pociągu
        exit(1);
    }
}

int main() {
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych

    // Tworzenie pamięci współdzielonej
    // int shmid = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
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

    // Generowanie pociągów
    int train_id = 0;
    while (1) {
        int priority = rand() % 3 + 1;      // Priorytet: 1 (ekspres), 2 (pospieszny), 3 (towarowy)
        int track = rand() % TRACKS_NUMBER;        // Kierunek: 0 (do tunelu), 1 (z tunelu)
        printf("Generating train ID=%d, priority=%d, track=%d\n", train_id, priority, track);

        create_train_process(train_id, priority, track, shm);
        train_id++;

        sleep(rand() % 3 + 1); // Czas pomiędzy generowaniem nowych pociągów
    }

    return 0;
}
