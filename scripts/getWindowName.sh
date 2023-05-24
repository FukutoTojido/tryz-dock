#!/bin/bash
# echo $1
xprop -id $1 "_NET_WM_NAME" | grep -Po --line-buffered "(?<=(_NET_WM_NAME\(UTF8_STRING\) = )).*" | sed -u 's:^.\(.*\).$:\1:;s/"/\\"/g' |\
xargs -0 -I{} eww update onHoverName={}