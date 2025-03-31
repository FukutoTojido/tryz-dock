#!/bin/bash
isOpen=`eww get open_panel`

# if [[ $(xrandr | grep " connected " | awk '{ print$1 }') == *"DP-0"* ]]; then
# 	MONITOR=1
# else
# 	MONITOR=0
# fi
MONITOR=$1

if [[ $isOpen == "true" ]]; then
    eww update open_panel=false;
    eww close panel 
else
    eww open panel --arg monitor=$MONITOR
    eww update open_panel=true;
fi