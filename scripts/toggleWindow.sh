#!/bin/bash
focused=`xdotool getwindowfocus`
# echo $1 $focused
if [[ $1 == $focused ]] ; then
	xdotool windowminimize $focused
    ./toggleWindow `xdotool getwindowfocus`
else
    xdotool windowactivate $1
fi