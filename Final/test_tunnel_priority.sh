#!/bin/bash

cleanup() {
    echo "Cleaning up..."
    pkill manager
    pkill generator
    rm -f generator.log
    rm -f trains.txt
    sleep 1
}

echo "Uruchamiam managera i generatora..."
./bin/manager > /dev/null 2>&1 &
MANAGER_PID=$!
sleep 1
./bin/generator > generator.log 2>&1 &
GENERATOR_PID=$!
sleep 5

echo "Testowanie logiki blokowania torów..."

sleep 10

grep "Train ID=" generator.log > trains.txt

declare -A track_priorities
track_priorities[0]=0
track_priorities[1]=0
track_priorities[2]=0
track_priorities[3]=0

while read -r line; do
    TRAIN_ID=$(echo "$line" | grep -oP 'Train ID=\K\d+')
    PRIORITY=$(echo "$line" | grep -oP 'priority=\K\d+')
    TRACK=$(echo "$line" | grep -oP 'track=\K\d+')

    if [[ -n "$TRAIN_ID" && -n "$PRIORITY" && -n "$TRACK" ]]; then
        if [[ ${track_priorities[$TRACK]} -lt $PRIORITY ]]; then
            track_priorities[$TRACK]=$PRIORITY
        fi
    fi
done < trains.txt

TUNNEL_TRAIN_ID=$(grep "Train ID=" generator.log | grep "entering tunnel" | tail -1 | grep -oP 'Train ID=\K\d+')

TUNNEL_TRAIN_PRIORITY=$(grep "Train ID=$TUNNEL_TRAIN_ID " generator.log | head -1 | grep -oP 'priority=\K\d+')
TUNNEL_TRAIN_TRACK=$(grep "Train ID=$TUNNEL_TRAIN_ID " generator.log | head -1 | grep -oP 'track=\K\d+')

if [ -z "$TUNNEL_TRAIN_ID" ]; then
    echo "FAIL: Żaden pociąg nie wjechał do tunelu"
    cleanup
    exit 1
fi

if [[ $TUNNEL_TRAIN_TRACK -lt 2 ]]; then
    OPPOSITE_SIDE_MAX_PRIORITY=${track_priorities[2]}
    OPPOSITE_SIDE_MAX_PRIORITY=$((OPPOSITE_SIDE_MAX_PRIORITY > track_priorities[3] ? OPPOSITE_SIDE_MAX_PRIORITY : track_priorities[3]))
else
    OPPOSITE_SIDE_MAX_PRIORITY=${track_priorities[0]}
    OPPOSITE_SIDE_MAX_PRIORITY=$((OPPOSITE_SIDE_MAX_PRIORITY > track_priorities[1] ? OPPOSITE_SIDE_MAX_PRIORITY : track_priorities[1]))
fi

echo "Pociąg w tunelu: ID=$TUNNEL_TRAIN_ID, Priorytet=$TUNNEL_TRAIN_PRIORITY, Tor=$TUNNEL_TRAIN_TRACK"
echo "Najwyższy priorytet na przeciwnej stronie: $OPPOSITE_SIDE_MAX_PRIORITY"

if [[ $OPPOSITE_SIDE_MAX_PRIORITY -eq 0 || $TUNNEL_TRAIN_PRIORITY -ge $OPPOSITE_SIDE_MAX_PRIORITY ]]; then
    echo "PASS: Poprawny pociąg wjechał do tunelu."
else
    echo "FAIL: Błędny pociąg wjechał do tunelu."
    cleanup
    exit 1
fi

cleanup
echo "Test zakończony pomyślnie!"
exit 0
