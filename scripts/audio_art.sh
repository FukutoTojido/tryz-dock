#!/bin/sh
AUDIO_PATH=$(playerctl metadata --format {{mpris:artUrl}} 2>&1 | grep -Eo "^file:///.*" | sed -E 's/file:\/\///')

if [[ -n $AUDIO_PATH ]]; then
    echo $AUDIO_PATH
else
    echo "~/.face"
fi
