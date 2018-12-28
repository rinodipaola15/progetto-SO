#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <sys/sem.h>
	#include <dirent.h>
	#include <sys/sysinfo.h>
	#include <pthread.h>

	#define N 400
	#define NMAX 400
	#define CMAX 400


	typedef struct ElemLista {
		char nome[400];
		int tipo;
		char path_elem[400];
		
	} TipoElemLista;

	
	typedef struct NodoLista {
		TipoElemLista info;
		struct NodoLista *next;
	} TipoNodoLista;


	typedef TipoNodoLista *TipoLista;


	void Inserimento_Ordinato_In_Lista (TipoLista *lis, char *elem1, int tipo, char *path) {
		TipoLista paux, prec, corr; 
		char path_aux[400];
		if((paux=(TipoLista)malloc(sizeof(TipoNodoLista)))==NULL) {
			printf("Errore nell'allocazione della memoria!\n");
			exit(1);
		}
 
		strcpy(path_aux, path);		
		strcpy(paux->info.nome, elem1);
		paux->info.tipo=tipo;
		strcat(path_aux, "/");
		strcat(path_aux, paux->info.nome);
		strcpy(paux->info.path_elem, path_aux);

		corr=*lis;
		prec=NULL;

		while(corr!=NULL && strcmp(corr->info.nome, elem1)<0) {
			prec=corr;
			corr=corr->next;
		}

		
		paux->next=corr;
			
		if(prec!=NULL)
			prec->next=paux;
		else
			*lis=paux;
	}


	void Visita_Lista(TipoLista lis) {

		while(lis!=NULL) {
			if(lis->info.tipo==1)
				printf("Directory: ");
			else
				printf("File: ");
			printf("%s", lis->info.nome);
			printf("\n");
			printf("Path: %s\n", lis->info.path_elem);
			lis=lis->next;
		}
	}


	
	TipoLista lis=NULL;

	pthread_mutex_t mutex_lista;	
	pthread_mutex_t mutex2_lista;

	int controlloterminaz[CMAX];

	


	TipoLista Trova_Dir(TipoLista lis) {
		while(lis!=NULL) {
			if(lis->info.tipo==1) {
				return lis;
			}
			else
				lis=lis->next;
		}
		return NULL;
	}


	
	void Elimina_Directory(TipoLista *lis, TipoElemLista elem) {
		TipoLista paux;
		if(*lis!=NULL)
			if(strcmp((*lis)->info.path_elem, elem.path_elem)==0) {
				paux=*lis;
				*lis=(*lis)->next;
				free(paux);
			}
		else
			Elimina_Directory(&(*lis)->next, elem);
	}



	void *funzione_thread(void *arg) {
		int tipo;
		char path_aux[400];
		DIR *dr;
		struct dirent *de;
		int ncore=(intptr_t)arg;
		int finito=0;
		TipoLista Esiste_Directory;
		while(!finito) {
			pthread_mutex_lock(&mutex_lista);
			Esiste_Directory=Trova_Dir(lis);
			if(Esiste_Directory!=NULL) {        //eliminiamo la directory
				Elimina_Directory(&lis, Esiste_Directory->info);
				Esiste_Directory->next=NULL;	
				pthread_mutex_unlock(&mutex_lista);
				// dobbiamo ancora inserire il finito=1
				strcpy(path_aux, Esiste_Directory->info.path_elem);
				pthread_mutex_lock(&mutex2_lista);
				controlloterminaz[ncore]=0;
				pthread_mutex_unlock(&mutex2_lista);
				dr = opendir(path_aux);
				while((de = readdir(dr)) != NULL) {
					if(de->d_type==DT_DIR) {
						tipo=1;    //directory
						Inserimento_Ordinato_In_Lista (&lis, de->d_name, tipo, path_aux);
					}
					else {
						tipo=0;    //file
						Inserimento_Ordinato_In_Lista (&lis, de->d_name, tipo, path_aux);
					}
				}							
			}
		}
	}

		
	
	
	int main() {

		char path[N];
		int ncore, i, res, tipo;
		pthread_t th[NMAX];
		struct dirent *de;

		pthread_mutex_init(&mutex_lista, NULL);
		pthread_mutex_init(&mutex2_lista, NULL);

		for(i=0; i<ncore; i++) {
			controlloterminaz[i]=0;
		}

		pthread_mutex_lock(&mutex_lista);

		printf("Inserisci il path assoluto:\n");
		scanf("%s", path);

		DIR *dr = opendir(path);

		if(dr==NULL)
			exit(EXIT_FAILURE);

		while((de = readdir(dr)) != NULL) {
			if(de->d_type==DT_DIR)
				tipo=1;    //directory
			else
				tipo=0;    //file
			Inserimento_Ordinato_In_Lista (&lis, de->d_name, tipo, path);
		}

		printf("Inserimento in lista effettuato\n");

		printf("Stampa delle directory e dei file:\n");

		closedir(dr);

		ncore=get_nprocs_conf();
		printf("Numero core: %d\n", ncore);


		for(i=0; i<ncore; i++) {
			res=pthread_create(&th[i], NULL, funzione_thread, (void *)(intptr_t)i);
			if(res!=0) {
				printf("Creazione del thread fallita\n");
				exit(EXIT_FAILURE);

			}

		}

		pthread_mutex_unlock(&mutex_lista);

		for(i=0; i<ncore; i++) {
			res=pthread_join(th[i], NULL);
			if(res!=0) {
				printf("Join fallita\n");
				exit(EXIT_FAILURE);

			}
		}

		Visita_Lista(lis);

		pthread_mutex_destroy(&mutex_lista);
		pthread_mutex_destroy(&mutex2_lista);
	
	}
