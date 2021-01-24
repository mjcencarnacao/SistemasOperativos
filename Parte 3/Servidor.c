#include "Settings.h"

Mensagem m;
Data* data;
int id, status, memory, semaphore;

// Resposta ao sinal SIGALRM, utilizado no procedimento wait, que irá auxiliar caso o Cliente cancele a consulta.

void responseALARM(int signal) {}

/* Armação e tratamento do sinal SIGINT (atalho <CTRL+C>).
   Após receber este sinal mostra no ecrã o valor atual dos counters(Perdidas, Tipo 1, Tipo 2, Tipo 3),
   encerrando o Servidor.
*/
void responseSIGINT(int signal) {
    printf("\nPerdidas: %d\nTipo 1: %d\nTipo 2: %d\nTipo 3: %d\n", data->save[0], data->save[1], data->save[2], data->save[3]);
    exit(0);
}

/* Procedimento operation.
   Este procedimento permite efetuar operações de DOWN e UP no array de semáforos utilizado.
   Neste projeto são utilizados dois semáforos: 1) Para registar uma consulta ( alterar e ler lista_consultas[10]);
                                                2) Para alterar o valor dos counters (save[4]).
*/
void operation(int id,int index,int value ) {
    struct sembuf s;
    s.sem_num = index;
    s.sem_op = value;
    s.sem_flg = 0;
    int status = semop ( id, &s, 1 );
    exit_on_error (status, "DOWN");
}

/* Procedimento start.
   1) Este procedimento permite verificar a existência da memória partilhada, caso exista estabelece a ligação.
      Caso a memória partilhada não exista, inicia-a (lista_consultas[index].tipo = -1).

   2) Permite criar/estabelecer ligação aos semáforos (semget) e inicializar os mesmos (semclt).
   
   3) Permite criar/estabelecer uma ligação á mailbox (msgget).
*/

void start() {
    memory = shmget( IPC_KEY, sizeof(Data), IPC_CREAT | IPC_EXCL | 0666);
    if(memory > 0) {
        data = (Data *) shmat(memory, 0, 0);
        exit_on_null(data, "shmat");
        printf("Memória partilhada iniciada.\n");
        for(int i = 0; i < 10; i++)
            data->lista_consultas[i].tipo = -1;
    } else {
        memory = shmget( IPC_KEY, sizeof(Data), IPC_CREAT | 0666);
        exit_on_error(memory, "Create");
        printf("Memória partilhada existente.\n");
        data = (Data *) shmat(memory, 0, 0);
        exit_on_null(data, "shmat");
    }
    semaphore = semget( IPC_KEY, 2, IPC_CREAT | 0666 );
    exit_on_error(semaphore, "semget");

    semctl(semaphore, 0, SETVAL, 1);
    semctl(semaphore, 1, SETVAL, 1);

    id = msgget( SRVKEY, 0666 | IPC_CREAT);
    exit_on_error(id, "Erro ao ligar á mailbox.");
}

/* Procedimento wait.
   Este procedimento permite verificar se a consulta é cancelada pelo Cliente.
   É usado um alarm para permitir que após o cancelamento da consulta por parte do Cliente o Servidor reconhece logo
   o pedido efetuado.
*/

void wait(int sala) {
        alarm(10);
        status = msgrcv( id, &m,sizeof(m.consulta), m.consulta.pid_consulta, 0);
        if(m.consulta.status == 5) {
            printf("Consulta cancelada pelo utilizador: %d\n", m.consulta.pid_consulta);
            data->lista_consultas[sala].tipo = -1;
            operation(semaphore, 0, 1);
            exit(0);
        }
}

/* Procedimento saveCounter.
   Este procedimento permite salvar o valor do tipo de consultas (Counters).
   Os counter estão guardados numa estrutura "Data" com a lista de consultas, 
   permitindo assim uma maior organização de acesso ás variáveis.
   na memória partilhada, tendo em atenção o valor dos semáforos (zona crítica).
*/

void saveCounter() {
    operation(semaphore, 1, -1);
    switch(m.consulta.tipo) {
    case 1 :
        data->save[1]++;
        break;
    case 2 :
        data->save[2]++;
        break;
    default:
        data->save[3]++ ;
        break;
    }
    operation(semaphore, 1, 1);
}

/* Procedimento update.
   1) Caso update = 2, permite enviar uma atualização do estado da consulta ao Cliente indicando o inicio de consulta na respetiva sala.

   2) Caso update = 3, permite enviar uma atualização do estado da consulta ao Cliente indicando o fim da consulta na respetiva sala.
   
   3) Caso update = 4, permite enviar uma atualização indicando que a consulta não é possível.
*/

void update(int update, int sala) {
    m.consulta.status = update;
    data->lista_consultas[sala] = m.consulta;
    m.tipo = m.consulta.pid_consulta;
    switch(update) {
    case 2:
        operation(semaphore, 0, 1);
        printf ("Consulta agendada para a sala %d\n", sala);
        break;
    case 3:
        printf ("Consulta terminada na sala %d\n", sala);
        operation(semaphore, 0, -1);
        data->lista_consultas[sala].tipo = -1;
        operation(semaphore, 0, 1);
        break;
    case 4:
        printf ("Lista de Consultas cheia.");
        break;
    default:
        printf("Erro ao atualizar.\n");
    }
    status = msgsnd(id, &m, sizeof(m.consulta), 0);
    exit_on_error(status, "Erro ao enviar");
}

int main() {
    signal(SIGINT, responseSIGINT);
    signal(SIGALRM, responseALARM);
    signal(SIGCHLD,SIG_IGN);
    start();
    while( 1 ) {
        status = msgrcv( id, &m,sizeof(m.consulta), 1, 0);
        exit_on_error (status, "Erro na recepção");
        if(m.consulta.status == 1) {
            printf("Chegou  novo  pedido  de  consulta  do  tipo  %d, descrição %s e PID %d\n", m.consulta.tipo, m.consulta.descricao, m.consulta.pid_consulta);
            if(fork() == 0) {
                operation(semaphore, 0, -1);
                for(int sala = 0; sala < 10; sala++) {
                    if(data->lista_consultas[sala].tipo == -1) {
                        update(2,sala);
                        wait(sala);
                        update(3,sala);
                        saveCounter();
                        break;

                    } else if(sala == 9) {
                        update(4,sala);
                        operation(semaphore, 1, -1);
                        data->save[0]++ ;
                        operation(semaphore, 1, 1);
                    }
                }
                exit(0);
            }
        }
    }
}
