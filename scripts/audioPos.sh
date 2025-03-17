#!/bin/sh
AUDIO_LENGTH="$(playerctl metadata --format {{mpris:length}} 2>&1 | grep -Eo "[0-9]+")"
AUDIO_POS="$(playerctl position)"

if [[ -n $AUDIO_LENGTH ]]; then
    echo $( echo "$AUDIO_POS * 1000000 * 100 / $AUDIO_LENGTH" | bc )
else
    echo "0"
fi