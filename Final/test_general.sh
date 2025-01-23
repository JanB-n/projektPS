#!/bin/bash

# Funkcja czyszcząca po teście
cleanup() {
    echo "Cleaning up..."
    pkill manager
    pkill generator
    sleep 1
}

# Test 1: Uruchomienie managera i generatora
echo "Test 1: Sprawdzenie uruchomienia procesów"
./bin/manager & 
MANAGER_PID=$!
sleep 1
./bin/generator &
GENERATOR_PID=$!
sleep 5

if pgrep -x "manager" > /dev/null && pgrep -x "generator" > /dev/null; then
    echo "PASS: Procesy manager i generator działają"
else
    echo "FAIL: Nie udało się uruchomić procesów"
    cleanup
    exit 1
fi

# Test 2: Sprawdzenie zakończenia działania generatora
echo "Test 2: Obsługa sygnału zakończenia (Ctrl+C)"
kill -INT $GENERATOR_PID
sleep 2

if ! pgrep -x "generator" > /dev/null; then
    echo "PASS: Generator zakończył działanie poprawnie"
else
    echo "FAIL: Generator nie zakończył działania"
    cleanup
    exit 1
fi

# Test 3: Sprawdzenie usuwania pamięci współdzielonej
echo "Test 3: Czyszczenie zasobów po zakończeniu"
kill -INT $MANAGER_PID
sleep 2

if ! ipcs -m | grep -q "12345"; then
    echo "PASS: Pamięć współdzielona usunięta"
else
    echo "FAIL: Pamięć współdzielona nie została usunięta"
    cleanup
    exit 1
fi

cleanup
echo "Wszystkie testy zakończone pomyślnie!"
exit 0
