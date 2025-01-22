#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_TRACKS 4
#define NUM_TRAINS 10
#define MAX_PRIORITY 3  // Ekspres - 1, Pospieszny - 2, Towarowy - 3

// Struktura opisująca pociąg
typedef struct {
    int id;                // Identyfikator pociągu
    int priority;          // Priorytet pociągu (1 - Ekspres, 2 - Pospieszny, 3 - Towarowy)
    char start_track;      // Początkowy tor (A, B, C, D)
    char end_track;        // Końcowy tor (A, B, C, D)
} Train;

// Struktura przechowująca stan torów
typedef struct {
    sem_t tracks[NUM_TRACKS];   // Semafory do synchronizacji torów
    Train *trains[NUM_TRACKS];  // Pociągi na torach A, B, C, D
    pthread_mutex_t mutex;      // Mutex do synchronizacji dostępu do wspólnej pamięci
} Tunnel;

// Funkcja inicjalizująca semafory i wspólną pamięć
void init_tunnel(Tunnel *tunnel) {
    for (int i = 0; i < NUM_TRACKS; i++) {
        sem_init(&tunnel->tracks[i], 0, 1); // Semafor dla każdego toru
        tunnel->trains[i] = NULL;  // Brak pociągów na torach początkowych
    }
    pthread_mutex_init(&tunnel->mutex, NULL);
}

// Funkcja do generowania pociągów
void *generate_trains(void *arg) {
    Tunnel *tunnel = (Tunnel *)arg;
    for (int i = 0; i < NUM_TRAINS; i++) {
        Train *train = malloc(sizeof(Train));
        train->id = i;
        train->priority = rand() % MAX_PRIORITY + 1;  // Losowy priorytet
        train->start_track = "ABCD"[rand() % NUM_TRACKS];  // Losowy tor startowy
        train->end_track = "ABCD"[rand() % NUM_TRACKS];    // Losowy tor docelowy
        
        printf("Generated Train %d, Priority: %d, Start: %c, End: %c\n", 
               train->id, train->priority, train->start_track, train->end_track);

        // Wstawienie pociągu na odpowiedni tor
        pthread_mutex_lock(&tunnel->mutex);
        for (int i = 0; i < NUM_TRACKS; i++) {
            if (tunnel->trains[i] == NULL) {
                tunnel->trains[i] = train;
                break;
            }
        }
        pthread_mutex_unlock(&tunnel->mutex);

        sleep(rand() % 2 + 1);  // Symulacja czasu oczekiwania na wygenerowanie nowego pociągu
    }
    return NULL;
}

// Funkcja sprawdzająca, który pociąg ma najwyższy priorytet
int get_highest_priority_train(Tunnel *tunnel) {
    int max_priority = MAX_PRIORITY + 1;
    int train_id = -1;

    for (int i = 0; i < NUM_TRACKS; i++) {
        if (tunnel->trains[i] != NULL && tunnel->trains[i]->priority < max_priority) {
            max_priority = tunnel->trains[i]->priority;
            train_id = i;
        }
    }
    return train_id;
}

// Funkcja zarządzająca wjazdem pociągów do tunelu
void *manage_trains(void *arg) {
    Tunnel *tunnel = (Tunnel *)arg;
    while (1) {
        // Sprawdzenie dostępności torów
        pthread_mutex_lock(&tunnel->mutex);

        int train_id = get_highest_priority_train(tunnel);
        if (train_id != -1) {
            Train *train = tunnel->trains[train_id];
            printf("Allowing Train %d to enter tunnel from track %c to track %c\n", 
                   train->id, train->start_track, train->end_track);
            tunnel->trains[train_id] = NULL; // Usuwamy pociąg z toru po wjeździe

            // Symulacja przejazdu przez tunel
            sleep(rand() % 3 + 2);
            printf("Train %d has passed through the tunnel\n", train->id);
        }

        pthread_mutex_unlock(&tunnel->mutex);
        sleep(1);  // Sprawdzenie raz na sekundę
    }
    return NULL;
}

// Funkcja główna
int main() {
    Tunnel tunnel;
    init_tunnel(&tunnel);

    pthread_t train_generator_thread, train_manager_thread;

    // Tworzenie wątku do generowania pociągów
    pthread_create(&train_generator_thread, NULL, generate_trains, &tunnel);
    // Tworzenie wątku do zarządzania pociągami
    pthread_create(&train_manager_thread, NULL, manage_trains, &tunnel);

    // Czekanie na zakończenie wątków
    pthread_join(train_generator_thread, NULL);
    pthread_join(train_manager_thread, NULL);

    return 0;
}
