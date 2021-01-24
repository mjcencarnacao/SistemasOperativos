#include "Consulta.h"

int accept;

/* C1) Nesta função é pedido ao utilizador uma descrição e um tipo (por scanf), atribuíndo assim
       os valores a uma Consulta X (o PID é obtido pela função getpid()), que irá ser usada, no 
       ponto C2) para criar o ficheiro "PedidoConsulta.txt". Caso o tipo não esteja entre [1,3]
       o programa sai com um exit code = 1.
*/
Consulta getInfo(){
    Consulta x;
    printf("Por favor introduza a descrição da Consulta: \n");
    scanf("%99[^\n]%*c", x.descricao);
    printf("Por favor indique o tipo de consulta (1-Normal, 2-COVID 19, 3-Urgente): \n");
    scanf("%i", &x.tipo);
    ((x.tipo > 3 || x.tipo < 1) ? exit(1) : "");
    x.pid_consulta = getpid();
    return x;}

/* C2) Nesta função é criado o ficheiro "PedidoConsulta.txt", registando o tipo; o PID do processo e a
       descrição atribuída á consulta x em C1). É verificado a existência do ficheiro, caso a
       consulta == NULL o programa sai com um exit code = 1.
*/
void createFile(Consulta x){
    FILE *consulta = fopen("PedidoConsulta.txt", "w");
    ((consulta == NULL) ? exit(1) : "");
    fprintf(consulta, "%d %d %s", x.tipo, x.pid_consulta, x.descricao);
    fclose(consulta);}

/* C3) Nesta resposta ao sinal SIGUSR1 ocorre a leitura do ficheiro "SrvConsultas.pid" para extração
       do PID do servidor. É verificada a integridade de SrvConsultas e caso seja igual a NULL o programa
       sai com um exit code = 1. Tendo em consideração a alínea C8) esta função é uma parte da resposta completa
       a SIGUSR1, sendo que, neste caso também é devolvido o PID do servidor.
*/
int responseSIGUSR1(){
    char buffer[1000]; int PID;
    FILE *SrvConsultas = fopen("SrvConsultas.pid", "r");
    ((SrvConsultas == NULL) ? exit(1) : "");
    while(fgets(buffer,sizeof(buffer), SrvConsultas)) PID = atoi(buffer);
    fclose(SrvConsultas);
    return PID;}

// C4) Verifica e indica o inicío de uma consulta, removendo o ficheiro "PedidoConsulta.txt" e igualando accept a 1.
void responseSIGHUP(int signal){
 printf("Consulta iniciada para o processo %d: \n", getpid());
 remove("PedidoConsulta.txt");
 accept = 1;}

// C5) Se a consulta foi iniciada, ou seja, accept = 1, é indicado a conclusão da mesma, terminando o processo com exit code = 0.
void responseSIGTERM(int signal){
    if(accept) printf("Consulta concluída para o processo %d: \n", getpid());
    exit(0);}

/* C6) Caso não existam salas disponíveis para realizar uma consulta é indicado que esta não é possível realizar-se, removendo o
       ficheiro "PedidoConsulta.txt" e terminando o processo com um exit code = 0.
*/
void responseSIGUSR2(int signal){
    printf("Consulta não é possível para o processo  %d: \n", getpid());
    remove("PedidoConsulta.txt");
    exit(0);}

// C7) Caso o Cliente queira terminar o pedido, pode o fazer com o atalho <CTRL+C>.
void responseSIGINT(int signal){
    printf("Paciente cancelou o pedido.\n");
    remove("PedidoConsulta.txt");
    exit(0);}

/* C8) Verifica a Existência do ficheiro "PedidoConsulta.txt", em caso afirmativo, este indica a sua existência, espera 10 
       segundos e volta a tentar efetuar o pedido de consulta. Em caso negativo envia o pedido de consulta de imediato, enviando 
       assim ao servidor um sinal SIGUSR1. 
*/
void validate(Consulta x){
    if(access("PedidoConsulta.txt", F_OK ) != -1) {
     printf("Erro. Ficheiro PedidoConsulta.txt já existe. Aguardando 10 segundos...\n");
     sleep(10);
     validate(x);}
    else {
     createFile(x);
     kill(responseSIGUSR1(), SIGUSR1);}}

// Função main() com as respostas pedidas aos sinais e a criação de um pedido de consulta. Uso de for infinito aguardando por sinais (pause()).
int main(){
    signal(SIGINT, responseSIGINT);
    signal(SIGHUP, responseSIGHUP);
    signal(SIGTERM, responseSIGTERM);
    signal(SIGUSR2, responseSIGUSR2);  
    Consulta x = getInfo(); 
    validate(x);   
    for(;;) pause();}
