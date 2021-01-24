#!/bin/bash

if [ ! -f medicos.txt ]; then
    touch medicos.txt
fi

#Checks all arguments 
if [[ $1 =~ [0-9] ]]; then
   echo "The field Name must not have any integers"
   exit -1
elif ! [[ "$2"  =~ ^[0-9]+$ ]]; then
   echo "Second argument must be an Integer."
   exit -1
elif [[ ! $4 == *"@"* ]]; then
   echo "Please enter a valid e-mail address."
   exit -1
elif [ "$#" -ne 4 ]; then
   echo "Script must have the following structure ./cria_medico.sh <Name> <License> <Specialty> <E-mail>"
   exit -1
fi

#Creates a Doctor
if ! grep -F $2 medicos.txt; then
    echo $1";"$2";"$3";"$4";0;0;0" >> medicos.txt
    cat medicos.txt
fi
