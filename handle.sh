#!/bin/bash

while true;
do
once=$(top -b -n1 | sed 1,7d)
while read line; do
    pid=$(echo $line | awk '{print $1}')
    cpu=$(echo $line | awk '{print $9}')
    process=$(readlink /proc/$pid/exe)

    if [[ -z $process ]]
    then
        continue
    fi

    echo $(date '+%X')","$pid","$cpu","$process >> $1
    sync
done <<< "$once"
done
