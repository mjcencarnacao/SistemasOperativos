#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct {
    int tipo;// Tipo de Consulta: 1-Normal, 2-COVID19, 3-Urgente
    char descricao[100]; // Descrição da Consulta
    int pid_consulta; // PID do processo que quer fazer a consulta
    } Consulta;