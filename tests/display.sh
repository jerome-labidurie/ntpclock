#!/bin/bash
# affiche alternativement des messages

# strings to display
# replace space by +
declare -a MSG=( Fablab+Lannion hello+world bonjour+les+gens )

while [ 1 ]
do
    for m in ${MSG[@]}
    do
        echo ${m/+/ }
        wget -q -O /dev/null "http://clock/?msg=${m}"
        sleep 30
    done
done