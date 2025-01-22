#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "common.h"

void create_train_process(int train_id, int priority, int direction, int track) {
    pid_t pid = fork();
    if (pid == 0) {
        char train_id_str[10], priority_str[10], direction_str[10], track_str[10];
        sprintf(train_id_str, "%d", train_id);
        sprintf(priority_str, "%d", priority);
        sprintf(direction_str, "%d", direction);
        sprintf(track_str, "%d", track);

        execlp("./train", "./train", train_id_str, priority_str, direction_str, track_str, NULL);
        perror("execlp failed");
        exit(1);
    }
}

int main() {
    srand(time(NULL));
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

    int train_id = 0;
    while (1) {
        int priority = rand() % 3 + 1;
        int direction = rand() % 2;
        int track = rand() % NUM_TRACKS;

        printf("Generating train ID=%d, priority=%d, direction=%d, track=%c\n",
               train_id, priority, direction, 'A' + track);

        create_train_process(train_id, priority, direction, track);
        train_id++;

        sleep(rand() % 3 + 1);
    }

    return 0;
}
