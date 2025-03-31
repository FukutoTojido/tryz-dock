#!/bin/bash

case "$1" in
    "reverse")
        if [[ ! -n $(eww active-windows | grep "window_switcher") ]]; then
            eww open window_switcher
        fi

        LIST_LENGTH=$(eww get window_order | jq length)
        INDEX=$((($(eww get index) - 1) % $LIST_LENGTH))

        if [ $INDEX == "-1" ]; then
            PID=$(eww get window_order | jq ".[$((LIST_LENGTH - 1))].pid")
            eww update index=$((LIST_LENGTH - 1))
            eww update selected_pid=$PID
        else
            PID=$(eww get window_order | jq ".[$INDEX].pid")
            eww update index=$INDEX
            eww update selected_pid=$PID
        fi
        echo $INDEX
        ;;
    "switch")
        if [[ ! -n $(eww active-windows | grep "window_switcher") ]]; then
            eww open window_switcher
        fi

        LIST_LENGTH=$(eww get window_order | jq length)
        INDEX=$((($(eww get index) + 1) % $LIST_LENGTH))
        PID=$(eww get window_order | jq ".[$INDEX].pid")
        eww update index=$INDEX
        eww update selected_pid=$PID
        echo $INDEX
        ;;
    "confirm")
        if [[ ! -n $(eww active-windows | grep "window_switcher") ]]; then
            exit
        fi
        INDEX=$(eww get index)
        PID=$(eww get window_order | jq ".[$INDEX].pid")
        $(dirname "$(realpath $0)")/toggleWindow.sh $PID
        eww update index=0
        eww close window_switcher
        echo $PID
        ;;
esac


