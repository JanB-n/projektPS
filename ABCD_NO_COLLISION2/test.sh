#!/bin/bash

echo "Test 1: Uruchamianie managera i generatora..."
./manager &
MANAGER_PID=$!
sleep 1
./generator &
GENERATOR_PID=$!

# Czekamy chwilę na działanie programu
sleep 5

# Sprawdzanie, czy procesy działają
if ps -p $MANAGER_PID > /dev/null && ps -p $GENERATOR_PID > /dev/null; then
    echo "Test 1 PASSED: Procesy działają poprawnie."
else
    echo "Test 1 FAILED: Procesy się nie uruchomiły."
fi

# Zabijanie procesów po teście
kill $MANAGER_PID $GENERATOR_PID
sleep 1

echo "Test 2: Sprawdzenie generowania pociągów..."

# Uruchomienie i sprawdzenie, czy generuje pociągi
./manager &
MANAGER_PID=$!
sleep 1
./generator > generator_output.txt &
GENERATOR_PID=$!
sleep 3
kill $MANAGER_PID $GENERATOR_PID

# Sprawdzanie wyników
if grep -q "Generated train" generator_output.txt; then
    echo "Test 2 PASSED: Pociągi generowane poprawnie."
else
    echo "Test 2 FAILED: Pociągi nie zostały wygenerowane."
fi

rm generator_output.txt
echo "Testy zakończone."
