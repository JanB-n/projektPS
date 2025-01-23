# Kompilator i flaga
CC = gcc
CFLAGS = -Wall -std=c99 -g -D_POSIX_C_SOURCE

# Ścieżki do plików źródłowych
SRC_DIR = src
BIN_DIR = bin
LOG_DIR = log

# Pliki źródłowe i obiektowe
SRC_GENERATOR = $(SRC_DIR)/generator.c
SRC_MANAGER = $(SRC_DIR)/manager.c
OBJ_GENERATOR = $(SRC_GENERATOR:.c=.o)
OBJ_MANAGER = $(SRC_MANAGER:.c=.o)

# Plik wykonywalny
GENERATOR_BIN = $(BIN_DIR)/generator
MANAGER_BIN = $(BIN_DIR)/manager

# Pliki testowe
TEST_SCRIPT1 = test_general.sh
TEST_SCRIPT2 = test_tunnel_priority.sh

# Cel domyślny - kompilacja i uruchomienie
all: $(GENERATOR_BIN) $(MANAGER_BIN)

# Kompilacja generatora
$(GENERATOR_BIN): $(OBJ_GENERATOR)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja managera
$(MANAGER_BIN): $(OBJ_MANAGER)
	$(CC) $(CFLAGS) -o $@ $^

# Kompilacja obiektów
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Uruchomienie testów
test: $(GENERATOR_BIN) $(MANAGER_BIN)
	./$(TEST_SCRIPT1)
	./$(MANAGER_BIN) > /dev/null 2>&1 &
	./$(GENERATOR_BIN) > $(LOG_DIR)/generator.log 2>&1 &
	sleep 5  # Poczekaj chwilę na uruchomienie
	./$(TEST_SCRIPT2)

# Sprzątanie
clean:
	rm -f $(OBJ_GENERATOR) $(OBJ_MANAGER) $(GENERATOR_BIN) $(MANAGER_BIN)
	rm -f $(LOG_DIR)/generator.log
	rm -f $(LOG_DIR)/test_output.txt

# Dodaj inne opcje kompilacji, jeśli wymagane
.PHONY: all test clean
