/**
 * @file common.h
 * @brief Definicje struktur danych i stałych dla systemu zarządzania ruchem pociągów w tunelu.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_TRAINS_ON_TRACK 3 /**< Maksymalna liczba pociągów na jednym torze */
#define TRACKS_NUMBER 4 /**< Liczba dostępnych torów */
#define SHM_KEY 12345 /**< Klucz do pamięci współdzielonej */


/**
 * @struct Train
 * @brief Struktura reprezentująca pociąg.
 */
typedef struct {
    int train_id; /**< ID pociągu */
    int priority; /**< Priorytet pociągu (1-3, 3 to najwyższy) */
    int track; /**< Numer toru, na którym znajduje się pociąg */
} Train;


/**
 * @struct Track
 * @brief Struktura reprezentująca tor.
 */
typedef struct {
    Train queue[MAX_TRAINS_ON_TRACK]; /**< Kolejka pociągów na torze */
    int queue_size; /**< Liczba pociągów w kolejce */
    int trains_to_dump; /**< Liczba pociągów do usunięcia */
    sem_t track_mutex; /**< Semafor kontrolujący dostęp do toru */
} Track;


/**
 * @struct SharedMemory
 * @brief Struktura przechowująca wspólne zasoby systemu.
 */
typedef struct {
    Track tracks[TRACKS_NUMBER]; /**< Tablica torów */
    int tunnel_busy; /**< Flaga zajętości tunelu */
    sem_t memory_mutex; /**< Semafor dostępu do pamięci współdzielonej */        
    sem_t tunnel_access; /**< Semafor dostępu do tunelu */
} SharedMemory;


