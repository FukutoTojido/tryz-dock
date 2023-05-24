#!/bin/bash
isOpen=`eww get calendarIsOpen`
if [[ $isOpen == "true" ]]; then
    eww update calendarIsOpen=false;
    eww close calendar
else
    eww update calendarIsOpen=true;
    eww open calendar
fi