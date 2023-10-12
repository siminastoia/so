#!/bin/bash

if [ $# -lt 3 ]
then
    echo "Usage : <numefisier><dir><file>n1 n2 n3"
     exit 1
fi
count=0
suma=0
wcount=0

filename=$1
dirname=$2

shift 2

for arg in "$@"
do
    if [ "$arg" -ge 10 ]
    then
	count=$(($count+1))
    fi
    suma=$(($suma+"$arg"))
done

echo "count: $count" > "$filename"
echo "suma: $suma" >> "$filename"

wcount=$((${#suma}))
echo "wcount: $wcount" >> "$filename" 

for file in "$dir"/*.txt
do

    if [ -f "$filename" ]
    then
	echo "continutul lui $filename: "
	cat "$filename"
    fi
done







