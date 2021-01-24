#!/bin/bash

echo -n "" > lista_negra_medicos.txt

while IFS='' read -r LINE || [ -n "${LINE}" ]; do
    consultas=$(echo "${LINE}" | cut -d ";" -f 6)
    rating=$(echo "${LINE}" | cut -d ";" -f 5)
    if (( consultas > 6 )) && (( rating < 5 )); then
        echo "${LINE}" >> lista_negra_medicos.txt
    fi
done < medicos.txt
cat lista_negra_medicos.txt


