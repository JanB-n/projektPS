#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_TRAINS 10 // Maksymalna liczba pociągów w kolejce na torze
#define NUM_TRACKS 4  // Liczba torów

typedef enum { EXPRESS = 3, FAST = 2, FREIGHT = 1 } Priority;
typedef enum { A, B, C, D } Track;

typedef struct {
    int id;
    Priority priority;
    Track origin;
    Track destination;
} Train;

typedef struct {
    Train queue[MAX_TRAINS];
    int front, rear, count;
    sem_t mutex;
    sem_t not_empty;
} TrackQueue;

TrackQueue tracks[NUM_TRACKS];
sem_t tunnel_mutex; // Kontrola dostępu do tunelu

void enqueue(TrackQueue *track, Train train) {
    sem_wait(&track->mutex);
    if (track->count < MAX_TRAINS) {
        track->queue[track->rear] = train;
        track->rear = (track->rear + 1) % MAX_TRAINS;
        track->count++;
    }
    sem_post(&track->mutex);
    sem_post(&track->not_empty);
}

Train dequeue(TrackQueue *track) {
    Train train;
    sem_wait(&track->not_empty);
    sem_wait(&track->mutex);
    train = track->queue[track->front];
    track->front = (track->front + 1) % MAX_TRAINS;
    track->count--;
    sem_post(&track->mutex);
    return train;
}

void *train_process(void *arg) {
    Train *train = (Train *)arg;

    // Pociąg chce wjechać do tunelu
    printf("Pociąg %d (priorytet: %d) z toru %d chce wjechać do tunelu.\n",
           train->id, train->priority, train->origin);

    // Czekaj na dostęp do tunelu
    sem_wait(&tunnel_mutex);
    printf("Pociąg %d wjeżdża do tunelu z toru %d.\n", train->id, train->origin);
    sleep(2); // Symulacja przejazdu przez tunel
    printf("Pociąg %d opuszcza tunel na tor %d.\n", train->id, train->destination);
    sem_post(&tunnel_mutex);

    free(train);
    return NULL;
}

void *manager_process(void *arg) {
    while (1) {
        Train *highest_priority_train = NULL;

        // Sprawdź wszystkie tory
        for (int i = 0; i < NUM_TRACKS; i++) {
            if (tracks[i].count > 0) {
                Train candidate = tracks[i].queue[tracks[i].front];
                if (!highest_priority_train || candidate.priority > highest_priority_train->priority) {
                    highest_priority_train = &tracks[i].queue[tracks[i].front];
                }
            }
        }

        if (highest_priority_train) {
            TrackQueue *origin_queue = &tracks[highest_priority_train->origin];
            Train train = dequeue(origin_queue);

            pthread_t thread;
            Train *train_copy = malloc(sizeof(Train));
            *train_copy = train;

            pthread_create(&thread, NULL, train_process, (void *)train_copy);
            pthread_detach(thread);
        }

        sleep(1); // Odczekaj chwilę przed kolejnym sprawdzeniem
    }
}

int main() {
    // Inicjalizacja kolejek torów
    for (int i = 0; i < NUM_TRACKS; i++) {
        tracks[i].front = 0;
        tracks[i].rear = 0;
        tracks[i].count = 0;
        sem_init(&tracks[i].mutex, 0, 1);
        sem_init(&tracks[i].not_empty, 0, 0);
    }

    // Inicjalizacja semafora tunelu
    sem_init(&tunnel_mutex, 0, 1);

    pthread_t manager_thread;
    pthread_create(&manager_thread, NULL, manager_process, NULL);

    // Generowanie pociągów
    int train_id = 1;
    while (1) {
        Train train;
        train.id = train_id++;
        train.priority = rand() % 3 + 1; // Priorytet 1-3
        train.origin = rand() % NUM_TRACKS;
        train.destination = rand() % NUM_TRACKS;

        while (train.destination == train.origin) {
            train.destination = rand() % NUM_TRACKS;
        }

        enqueue(&tracks[train.origin], train);
        printf("Wygenerowano pociąg %d (priorytet: %d) z toru %d do toru %d.\n",
               train.id, train.priority, train.origin, train.destination);

        sleep(rand() % 3 + 1); // Symulacja czasu między pociągami
    }

    pthread_join(manager_thread, NULL);

    // Sprzątanie
    sem_destroy(&tunnel_mutex);
    for (int i = 0; i < NUM_TRACKS; i++) {
        sem_destroy(&tracks[i].mutex);
        sem_destroy(&tracks[i].not_empty);
    }

    return 0;
}
