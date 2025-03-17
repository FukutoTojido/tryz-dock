#!/bin/bash
isOpen=`eww get calendarIsOpen`

if [[ $(xrandr | grep " connected " | awk '{ print$1 }') == *"DP-0"* ]]; then
	MONITOR=1
else
	MONITOR=0
fi


if [[ $isOpen == "true" ]]; then
    eww update calendarIsOpen=false;
    eww close calendar 
else
    eww update calendarIsOpen=true;
    eww open calendar --arg monitor=$MONITOR
fi