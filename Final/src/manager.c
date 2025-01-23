#include "common.h"

int shmid;
SharedMemory* shm;

void cleanup() {
    printf("\nCleaning manager...\n");
    
    for (int i = 0; i < TRACKS_NUMBER; i++) {
        sem_destroy(&shm->tracks[i].track_mutex);
    }
    sem_destroy(&shm->memory_mutex);
    sem_destroy(&shm->tunnel_access);
    
    if (shmdt(shm) == -1) {
        perror("shmdt failed");
    }
    
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
    } else {
        printf("Finished cleaning manager.\n");
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
        sem_wait(&shm->memory_mutex);
            int valid_trains[TRACKS_NUMBER] = {0};

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

            printf("Trains waiting:\n");
            for (int i = 0; i < TRACKS_NUMBER; i++) {
                if (valid_trains[i]) {
                    printf("Track %d: Train ID=%d, Priority=%d\n", i, shm->tracks[i].queue[0].train_id, shm->tracks[i].queue[0].priority);
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

            sem_post(&shm->tunnel_access);
            sem_post(&shm->tracks[track_to_move].track_mutex);
            printf("Decided - Track %d: Train ID=%d, Priority=%d\n", 
                track_to_move, shm->tracks[track_to_move].queue[0].train_id, shm->tracks[track_to_move].queue[0].priority);

            for (int i = 0; i < shm->tracks[track_to_move].queue_size - 1; i++) {
                shm->tracks[track_to_move].queue[i] = shm->tracks[track_to_move].queue[i + 1];
            }
            shm->tracks[track_to_move].queue_size--;
        sem_post(&shm->memory_mutex); 
        sleep(3);
    }
}

int main() {
    signal(SIGINT, cleanup);
    shmid = shmget(SHM_KEY, sizeof(SharedMemory), IPC_CREAT | 0666);
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
    sem_init(&shm->memory_mutex, 1, 1);   
    sem_init(&shm->tunnel_access, 1, 0);


    printf("Tunnel manager started.\n");

    process_queue(shm);

    return 0;
}
