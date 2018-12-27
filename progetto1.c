//CONSEGNA 1 SISTEMI OPERATIVI
#include<stdio.h>
#include<stdlib.h>
#include<sys/sem.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<sys/sysinfo.h>
#include<sys/stat.h>
#include<pthread.h>
#define N 200

//creo la lista
typedef struct lista{
char nome[64];
int tipo;
char path[200];
}TipoElemLista;

typedef struct nodolista{
TipoElemLista info;
struct nodolista *next;
}Tiponodolista;

typedef Tiponodolista* Tipolista;

void inserimentoordinatoinlista(Tipolista *lis, char* elem, int elem2, char* path){
Tipolista paux, prec, cor;
char pathaux[200];
if((paux = (Tipolista)malloc(sizeof(Tiponodolista))) == NULL){
perror("ERRORE ALLOCAZIONE MEMORIA");
exit(1);
}

strcpy(paux->info.nome, elem);
paux->info.tipo = elem2;
strcpy(pathaux, path);
strcat(pathaux,"/");
strcat(pathaux, paux->info.nome);
strcpy(paux->info.path, pathaux);
cor = *lis;
prec = NULL;
while(cor != NULL && strcmp(cor->info.nome, elem) < 0){
prec = cor;
cor = cor->next;
}

paux->next = cor;

if(prec != NULL)
prec->next = paux;
else
*lis=paux;
}

void visitalista(Tipolista lis){
while(lis != NULL){
printf("%s \n", lis->info.nome);
if (lis->info.tipo == 1){
printf("Directory\n");
}
printf("%s\n", lis->info.path);
lis = lis->next;
}
}

Tipolista lis = NULL;
pthread_mutex_t mutex_lista;
void *funzione_thread(void*arg);

void scansionalista(Tipolista *lis){
DIR *dr;
int tipo;
struct dirent *de;
char pathaux[200];
while((*lis) != NULL){
strcpy(pathaux,(*lis)->info.path);
if((*lis)->info.tipo == 1){
dr = opendir(pathaux);
while((de = readdir(dr)) != NULL){
//salvo l'elemento in una lista e incremento la lista
if(de->d_type == DT_DIR){
tipo = 1;
inserimentoordinatoinlista(&(*lis), de->d_name, tipo, pathaux);
scansionalista(&(*lis));
}else{
tipo = 0;
inserimentoordinatoinlista(&(*lis), de->d_name, tipo, pathaux);
}
}//while
}//if
(*lis) = (*lis)->next;
}//while
}

int main(void){

//utilizzo una opendir e una readdir
//con opendir apre la directory passata da tastiera e salvata in una stringa e ritorna un puntatore allo stream della directory, questa funzione permette un list di file.

int Ncores;
int i;
int res;
pthread_t th;
int tipo;
int thread;
int risultato;

struct dirent *de;
//apre la direcory corrente dove mi trovo e la assegna ad un puntatore
DIR *dr;
char path[N];
printf("inserisci il path assoluto: \n");
scanf("%s", path);
dr = opendir(path);
//tramite readdir stampo i vari campi della lista essendo un while
while((de = readdir(dr)) != NULL){
//salvo l'elemento in una lista e incremento la lista
if(de->d_type == DT_DIR){
tipo = 1;
}else{
tipo = 0;
}
inserimentoordinatoinlista(&lis, de->d_name, tipo, path);
}
printf("\n ******STAMPA*******\n");
visitalista(lis);
printf("INSERIMENTO EFFETTUATO!\n");
closedir(dr);

//così vedo il numero di core
Ncores = get_nprocs_conf();
printf("%d\n", Ncores);

for(i=0;i<Ncores;i++){
res = pthread_create(&th, NULL, funzione_thread, i);
if (res != 0){
perror("CREAZIONE FALLITA\n");
exit(EXIT_FAILURE);
}//if
}//for
//for(i=0; i<Ncores; i++){
if((pthread_join(th, NULL)) != 0){
perror("Errore");
exit(EXIT_FAILURE);
}
visitalista(lis);
//}
}

void* funzione_thread(void* arg){
//printf("ok");
pthread_mutex_lock (& mutex_lista);
//visitalista(lis);
scansionalista(&lis);
pthread_mutex_unlock (& mutex_lista);
}
