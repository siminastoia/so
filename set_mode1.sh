#!/bin/bash

if [ $# -ne 2 ];
then
    echo "use: $0 <director> <caracter>"
    exit 1
fi

if [ ! -d "$1" ];
then
    echo "Directorul $1 nu exista"
    exit 1
fi

director=$1
caracter=$2

#nerecursiv 
for file in "$1"/*.txt; do
    if [ -f "$file" ]; then
	if [ "$2" = "r" ]
	then
	    chmod +r "$file"
	    echo "read"
	elif [ "$2" = "w" ]
	then
	    chmod +w "$file"
	    echo "write"
	elif [ "$2" = "x" ]
	then
	    chmod +x "$file"
	    echo "execute"
	else
	    echo "Caracterul este invalid, acesta trebuie sa fie r, w sau x"
	    exit 1
	fi
	
    fi
    echo "Drepturile au fost actrualizate"
done

#recursiv
for entry in "$1"/*; do
    if [ -d "$entry" ];
    then
	bash "$0" "$entry" "$2"
    fi
done

	
