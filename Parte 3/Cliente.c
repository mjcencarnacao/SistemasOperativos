#include "Settings.h"

Mensagem m;
int id, status, accept;

/* C1) Nesta procedimento é pedido ao utilizador uma descrição e um tipo (por scanf) (o PID é obtido pela função getpid()), 
       permitindo assim que os valores sejam enviados para o Servidor.
       Caso o tipo não esteja entre [1,3] o programa sai com um exit code = 1.
*/

void getInfo() {
    m.tipo = 1;
    m.consulta.pid_consulta = getpid();
    m.consulta.status = 1;
    printf("Por favor introduza a descrição da Consulta: ");
    scanf("%99[^\n]%*c", m.consulta.descricao);
    printf("Por favor indique o tipo de consulta (1-Normal, 2-COVID 19, 3-Urgente): ");
    scanf("%i", &m.consulta.tipo);
    ((m.consulta.tipo > 3 || m.consulta.tipo < 1) ? exit(1) : "");
}
/* Procedimento checkStatus.
   Este procedimento, utilizando um switch, permite verificar os diferentes estados que a Consulta (dentro da Mensagem m) irá ter.
   É invocado na main() dentro de um while(1) após a receção de uma mensagem, atualizando assim a shell do Cliente. 
   2) Consulta iniciada.
   3) Consulta concluída.
   4) Consulta não é possível.

*/
void checkStatus() {
    switch(m.consulta.status) {
    case 2:
        printf("Consulta iniciada para o processo: %d \n", getpid());
        accept = 1;
        break;
    case 3:
        if(accept) {
            printf("Consulta concluída para o processo: %d \n", getpid());
            exit(0);
        } else
            exit_on_error (accept, "Erro, a consulta nõ foi iniciada.");
        break;
    case 4:
        printf("Consulta não é possível para o processo: %d \n", getpid());
        exit(0);
        break;

    default:
        printf("Erro ao aceder ao status.\n");
    }
}

/* C7) Caso o Cliente queira terminar o pedido, pode o fazer com o atalho <CTRL+C>.
       Envia uma mensagem com estado = 5, indicando ao Servidor o cancelamento deste.
*/

void responseSIGINT(int signal) {
    printf("\nPaciente cancelou o pedido.\n");
    m.tipo = getpid();
    m.consulta.status = 5;
    status = msgsnd(id, &m, sizeof(m.consulta), 0);
    exit_on_error (status, "Erro ao comunicar com o servidor.");
    exit(0);
}

int main() {
    signal(SIGINT, responseSIGINT);

    // Ligarção ao servidor
    id = msgget( SRVKEY, 0 );
    exit_on_error(id, "Não foi possível estabelecer uma conexão ao Servidor.");

    //Envio da Mensagem
    getInfo();
    status = msgsnd(id, &m, sizeof(m.consulta), 0);
    exit_on_error(status, "Erro ao enviar.");
    printf("A mensagem foi enviada\n");

    while(1) {
        //Receção da Mensagem
        status = msgrcv(id, &m, sizeof(m.consulta), m.consulta.pid_consulta, 0);
        exit_on_error(status, "erro ao receber");
        checkStatus();

    }
}

