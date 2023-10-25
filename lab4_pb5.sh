#!/bin/bash
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <director> <output_file>"
    exit 1
fi

director="$1"
output_file="$2"
total_caractere=0

cd "$director" || { echo "Directorul $director nu exista!"; exit 1; }

for fisier in *.txt; do
    if [ -f "$fisier" ]; then
        numar_caractere=$(wc -c < "$fisier")
        total_caractere=$((total_caractere + numar_caractere))
        echo "$fisier $numar_caractere" >> "$output_file"
    fi
done

echo "TOTAL $total_caractere" >> "$output_file"
echo " Rezultatele au fost salvate in $output_file."
