CC = gcc
CFLAGS = -Wall -std=c99 -g -D_POSIX_C_SOURCE

SRC_DIR = src
BIN_DIR = bin
LOG_DIR = log

SRC_GENERATOR = $(SRC_DIR)/generator.c
SRC_MANAGER = $(SRC_DIR)/manager.c
OBJ_GENERATOR = $(SRC_GENERATOR:.c=.o)
OBJ_MANAGER = $(SRC_MANAGER:.c=.o)

GENERATOR_BIN = $(BIN_DIR)/generator
MANAGER_BIN = $(BIN_DIR)/manager

TEST_SCRIPT1 = test_general.sh
TEST_SCRIPT2 = test_tunnel_priority.sh

all: $(GENERATOR_BIN) $(MANAGER_BIN)

$(GENERATOR_BIN): $(OBJ_GENERATOR)
	$(CC) $(CFLAGS) -o $@ $^

$(MANAGER_BIN): $(OBJ_MANAGER)
	$(CC) $(CFLAGS) -o $@ $^

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(GENERATOR_BIN) $(MANAGER_BIN)
	./$(TEST_SCRIPT1)
	./$(MANAGER_BIN) > /dev/null 2>&1 &
	./$(GENERATOR_BIN) > $(LOG_DIR)/generator.log 2>&1 &
	sleep 5  # Poczekaj chwilę na uruchomienie
	./$(TEST_SCRIPT2)

clean:
	rm -f $(OBJ_GENERATOR) $(OBJ_MANAGER) $(GENERATOR_BIN) $(MANAGER_BIN)
	rm -f $(LOG_DIR)/generator.log
	rm -f $(LOG_DIR)/test_output.txt

.PHONY: all test clean
