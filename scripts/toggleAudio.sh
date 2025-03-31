#!/bin/bash
isOpen=`eww get open_audio`

# if [[ $(xrandr | grep " connected " | awk '{ print$1 }') == *"DP-0"* ]]; then
# 	MONITOR=1
# else
# 	MONITOR=0
# fi
MONITOR=$1

if [[ $isOpen == "true" ]]; then
    eww update open_audio=false;
    eww close playing 
else
    eww open playing --arg monitor=$MONITOR
    eww update open_audio=true;
fi