#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include <signal.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define exit_on_error(s,m)if( s < 0 ) { perror(m); exit(1); }
#define exit_on_null(s,m) if (s==NULL) { perror(m); exit(1); }

#define SRVKEY 0x0a92628
#define MSGKEY 0x0a92628
#define IPC_KEY 0x0a92628

typedef struct {
    int tipo; // Tipo de Consulta: 1-Normal, 2-COVID19, 3-Urgente
    char descricao[100]; // Descrição da Consulta
    int pid_consulta;// PID do processo que quer fazer a consulta
    int status; // Estado da consulta: 1-Pedido, 2-Iniciada 3-Terminada, 4-Recusada, 5-Cancelada
} Consulta;


typedef struct {
    long tipo;
    Consulta consulta;
} Mensagem;

typedef struct {
    Consulta lista_consultas[10];
    int save[4];
} Data;
