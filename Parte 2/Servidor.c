#include "Consulta.h"

Consulta lista_consultas[10];
int save[4];
FILE *StatsConsultas;

/* S1) Primeiro passo para inicializar o Servidor.
       Nesta função é permitida a leitura, caso exista (é indicada a sua existência), do ficheiro binário "StatsConsultas.dat", 
       que contem os dados da execução anterior (Consultas perdidas e dos diferentes tipos), usando um fread e um array "save". 
       Inicia, também, todos os campos da lista de consulta a -1 (sala desocupada)
*/
void start(){
    StatsConsultas = fopen("StatsConsultas.dat", "rb");
    if(StatsConsultas == NULL) printf("\nFicheiro StatsConsulta.dat inexistente.\n"); 
    else{printf("\nFicheiro StatsConsulta.dat detetado.\n");
         fread(save,sizeof(int),4,StatsConsultas); 
         fclose(StatsConsultas);}
    for(int i = 0; i < 10; i++)
        lista_consultas[i].tipo = -1;}

/* S2) Segundo passa para inicializar o Servidor.
       Nesta função, é efetuado o registo do PID, do Servidor, num ficheiro "SrvConsultas.pid" que irá, mais tarde, ser lido pelo cliente
       para permitir o envio de sinais ao PID do processo (Servidor.c) correto. É verificada se o ficheiro em questão é igual a NULL,sendo que nesse caso o programa
       é terminado com um exit code = 1, caso contrário, é indicado que o Servidor está Online, pronto a receber pedidos de consulta.
*/
void registry(){
FILE *SrvConsultas = fopen("SrvConsultas.pid", "w");
    ((SrvConsultas == NULL) ? exit(1) : "");
    fprintf(SrvConsultas, "%d", getpid());
    fclose(SrvConsultas);
    printf("\nServidor Online. \n");}

/* S3) Resposta a SIGUSR1.
       Nesta resposta permitimos a leitura do pedido de consulta efetuado pelo Cliente (ficheiro "PedidoConsulta.txt" que contem o tipo, PID e descrição),
       onde, os seus constituintes são extraidos usando um fscanf. É criado uma Consulta Y com os items indicados no ficheiro, onde irá ser colocado na próxima
       posição livre do array lista_consulta[] (livre = -1), indicando que a consulta foi agendada para a sala x (incrementando o tipo de consulta no array save[]). 
       É criado um processo filho, no qual são enviados, ao Cliente, os sinais SIGHUP e SIGTERM, indicando o início e fim da consulta (após 10 segundos (sleep(10))),
       onde por fim, a sala é desocupada (igual a -1).
*/
void responseSIGUSR1(int signal){
    char descricao[100]; int tipo, pid, i;
    FILE *x = fopen("PedidoConsulta.txt", "r");
    ((x == NULL) ? exit(1) : "");
    fscanf(x, "%d %d %[^\n]s", &tipo, &pid, &descricao);
    fclose(x);
    Consulta y = {tipo, "" , pid};
    strcpy(y.descricao, descricao);
    printf("Chegou  um novo  pedido  de  consulta  do tipo: %d descrição: %s PID: %d\n", tipo, descricao, pid);

    for(i = 0; i < 10; i++)
    if((lista_consultas[i].tipo != -1) && i == 9){
       printf("Lista de consultas cheia. \n");
       kill(pid,SIGUSR2);
       save[0]++;}
    
    for(i = 0; i < 10; i++)
        if (lista_consultas[i].tipo == -1){
          printf("Consulta agendada para a sala %d.\n" , i);
          lista_consultas[i] = y;
          switch(tipo) {
          case 1 : save[1]++; break;
          case 2 : save[2]++; break;
          default: save[3]++ ;break;}
          break;} 

   if(fork() == 0){
       kill(pid, SIGHUP);
       sleep(10);
       printf("Consulta terminada na sala %d.\n", i);
       kill(pid, SIGTERM);
       exit(0);}
   wait(NULL);
   lista_consultas[i].tipo = -1;}

/* S4) Resposta a SIGINT.
       Nesta função, após a introdução da combinação CTRL + C (terminar o processo Servidor), são incrementadas e registadas
       as consultas(perdidas, tipo 1, tipo 2 e tipo 3) dentro de um ficheiro binário "StatsConsultas.dat" usando um fwrite com o array save[]
       Caso tudo se verifique bem o programa termina com um exit code = 0, caso contrário, termina com exit code = 1 (StatsConsultas == NULL).
       Este ficheiro pode ser lido na CLI com o comando: hexdump -e '4 "%i\t" "\n"' StatsConsultas.dat
*/
void responseSIGINT(int signal){
remove("PedidoConsulta.txt");
    StatsConsultas = fopen("StatsConsultas.dat", "wb");
    ((StatsConsultas == NULL) ? exit(1) : "");
    fwrite(save, sizeof(int), 4, StatsConsultas);
    fclose(StatsConsultas);
    exit(0);}

// Função main() com as respostas pedidas aos sinais e a criação de um pedido de consulta. Uso de for infinito aguardando por sinais (pause()).
int main(){
    signal(SIGUSR1,responseSIGUSR1);
    signal(SIGINT,responseSIGINT);
    start();
    registry();
    for(;;) pause();}