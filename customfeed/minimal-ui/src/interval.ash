#!/bin/sh

i=0
while true
do 
    echo "no data: $i second past <br/>" >> /tmp/tmpdata
    sleep 5
    i=$((i+5))
done

