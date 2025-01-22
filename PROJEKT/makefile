# Nazwa plików wykonywalnych
TARGETS = manager generator train

# Kompilator
CC = gcc

# Flagi kompilacji
CFLAGS = -Wall -Wextra -pthread -lrt

# Pliki nagłówkowe
INCLUDES = common.h

# Pliki źródłowe
SRCS = manager.c generator.c train.c

# Pliki obiektowe
OBJS = $(SRCS:.c=.o)

# Domyślny cel - kompilacja i uruchomienie
all: $(TARGETS) run_all

# Kompilacja poszczególnych programów
manager: manager.o
	$(CC) $(CFLAGS) -o manager manager.o

generator: generator.o
	$(CC) $(CFLAGS) -o generator generator.o

train: train.o
	$(CC) $(CFLAGS) -o train train.o

# Reguła kompilacji plików źródłowych na obiektowe
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

# Usunięcie plików wykonywalnych i obiektowych
clean:
	rm -f $(TARGETS) *.o

# Uruchomienie wszystkich procesów w tle na jednym terminalu
run_all: all
	@echo "Starting manager..."
	./manager & sleep 1
	@echo "Starting generator..."
	./generator & sleep 1
	@echo "Simulation running. Press CTRL+C to stop."
	wait

# Automatyczna rekompilacja, jeśli pliki się zmienią
.PHONY: all clean run_all
