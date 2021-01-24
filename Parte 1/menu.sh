#!/bin/bash

#Checks for arguments
if [ "$#" -ne 0 ]; then
   echo "The script does not need any arguments."
   exit -1
fi

curl -s "http://artscene.textfiles.com/vt100/movglobe.vt"|pv -q -L 9600 
while : ; do
echo -e "1. Create Patients\n2. Create Doctor\n3. Stats\n4. Rate Doctor\n0. Exit"
read -p "What would you like to do? : " option
  case $option in
	  1) ./cria_pacientes.sh;;
     2) read -p "Name: " nome
        read -p "License: " licensa
        read -p "Specialty: " special
        read -p "Email: " email
        ./cria_medico.sh "$nome" $licensa "$special" $email;;
     3) read -p "Patient Location: " localidade
        read -p "Balance: " saldo
        ./stats.sh $localidade $saldo;;
     4) ./avalia_medicos.sh ;;
     0) exit 0;;
  esac
done