#!/bin/bash

#Checks arguments
if [[ $1 =~ [0-9] ]]; then
   echo "The field Location must not have any integers"
   exit -1
elif ! [[ "$2"  =~ ^[0-9]+$ ]]; then
   echo "Second argument must be an Integer."
   exit -1
elif [ "$#" -ne 2 ]; then
   echo "Script must have the following structure ./stats.sh <Location> <Balance>"
   exit -1
fi


#Result
echo "There are: `grep $1 pacientes.txt | wc -l` Patients from "$1"."
echo "There are: `awk -v y="$2" '{split($0,array,";"); if(array[7]>y)print y}' medicos.txt| wc -l` Doctors with more than "$2" balance."
exit 0