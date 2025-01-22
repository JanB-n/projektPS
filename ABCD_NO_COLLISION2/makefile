# Kompilator
CC = gcc

# Flagi kompilacji
CFLAGS = -pthread -lrt -Wall -Wextra -std=c11

# Nazwy plików źródłowych i wynikowych
SOURCES = manager.c generator.c
EXECUTABLES = manager generator

# Domyślny cel (kompilacja wszystkiego)
all: $(EXECUTABLES)

# Kompilacja managera
manager: manager.c shared.h
	$(CC) $(CFLAGS) -o manager manager.c

# Kompilacja generatora
generator: generator.c shared.h
	$(CC) $(CFLAGS) -o generator generator.c

# Uruchamianie aplikacji
run: all
	@echo "Uruchamianie menedżera w tle..."
	./manager &
	sleep 1
	@echo "Uruchamianie generatora..."
	./generator

# Testowanie programu
test: all
	@echo "Uruchamianie testów..."
	bash test.sh

# Czyszczenie plików binarnych
clean:
	rm -f $(EXECUTABLES) *.o
	@echo "Wyczyszczono pliki wynikowe."

# Ponowna kompilacja wszystkiego
rebuild: clean all
