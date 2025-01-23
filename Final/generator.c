#include "common.h"

pid_t train_pids[MAX_TRAINS_ON_TRACK * TRACKS_NUMBER];
int train_count = 0;

void cleanup(int signum) {
    printf("\nCleaning generator...\n", signum);

    for (int i = 0; i < train_count; i++) {
        printf("Stopping train process %d...\n", train_pids[i]);
        kill(train_pids[i], SIGTERM);
    }

    printf("Finished cleaning generator.\n");
    exit(0);
}

void create_train_process(int train_id, int priority, int direction) {
    pid_t pid = fork();
    if (pid == 0) { 
        char train_id_str[10], priority_str[10], direction_str[10];
        sprintf(train_id_str, "%d", train_id);
        sprintf(priority_str, "%d", priority);
        sprintf(direction_str, "%d", direction);

        execlp("./train", "./train", train_id_str, priority_str, direction_str, NULL);
        perror("execlp failed"); 
        exit(1);
    }
    else if (pid > 0) {
        if (train_count < MAX_TRAINS_ON_TRACK * TRACKS_NUMBER) {
            train_pids[train_count++] = pid;
        }
    } else {
        perror("fork failed");
    }
}

int main() {
    signal(SIGINT, cleanup);
    srand(time(NULL)); 

    // Generowanie pociągów
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
