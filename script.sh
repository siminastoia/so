#!/bin/bash

# Verifica daca exista un singur argument
if [ "$#" -ne 1 ]; then
    echo "Utilizare: $0 <caracter>"
    exit 1
fi

# Stocam caracterul primit ca argument
caracter=$1

# Contor pentru propozitiile corecte
corecte=0

while IFS= read -r line; do
    # Verifica daca linia incepe cu litera mare
    if [[ $line =~ ^[A-Z] ]]; then
        # Verifica daca linia contine doar caractere valide si se termina corespunzator
        if [[ $line =~ ^[A-Za-z0-9,.\!\?\ ]+[\.!\?]$ && ! $line =~ ,\ (si|Si) ]]; then
            # Verifica daca linia contine caracterul dat ca argument
            if [[ $line == *"$caracter"* ]]; then
                ((corecte++))
            fi
        fi
    fi
done

# Afiseaza numarul de propozitii corecte
echo "$corecte"
