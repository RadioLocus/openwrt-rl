#!/bin/sh

cut_var=1
timeout $1 tcpdump -i mon0 ether host $2 | grep dB | awk -F'dB' -v my_var="$cut_var" '{print $my_var}' | awk '{print $(NF),"<br>"}' | grep -n - >> /tmp/tmpdata


