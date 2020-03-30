/*
   Fichero: Batracios.c
   Integrantes del grupo:	Pablo Jesús González Rubio - i0894492
                        Sergio García González - i0921911
   Fecha de modificación: 30/03/2020
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "batracios.h"
#include <string.h>
#include <locale.h>


/* ============= Variables Globales ============= */

char *ptr;
int mem, sem;
int *r_salvadas, *r_nacidas, *r_perdidas;
int noTerminado=1;
struct sembuf sems;
struct sigaction ac;

// Unión necesario para la ejecución del programa en encina
union semun {
	int val;
	struct semid_ds *buf;
};

/* ============= Funciones ============= */

void rana(int nRana);
void crias(int nProcesos, int nRana);
void intHandler(int a);
void finPrograma();

/* ============= Inicio ============= */

int main (int argc, char*argv[]){
	setlocale(LC_ALL, "");
	int lTroncos[]={4,5,4,5,4,5,4},lAguas[]={5,4,3,5,3,4,5};
	int dirs[]={1,0,1,0,1,0,1};
	int nRana, nProcesos, nTroncos, param1, param2;
	int *movX, *movY;
	char errorLineaOrdenes[] = "USO: ./batracios VELOCIDAD VELOCIDAD_PARTO\n";
	union semun sem1, sem2, sem3, sem4, sem5, sem6;

	sem1.val=25;
	sem2.val=1;
	sem3.val=1;
	sem4.val=1;
	sem5.val=1;
	sem6.val=1;

/* ============= Comprobacion de parametros ============= */

	if(argc != 3) {
		write(2,errorLineaOrdenes,strlen(errorLineaOrdenes));
		exit(1);
	}

	param1 = atoi(argv[1]);
	param2 = atoi(argv[2]);

	if(param1 < 0 || param1 > 1000) {
		write(2, "Ha introducido una velocidad incorrecta.\nPor favor, introduzca una velocidad ente 0 y 1000", strlen("Ha introducido una velocidad incorrecta.\nPor favor, introduzca una velocidad ente 0 y 1000"));
		exit(1);
	}

	if (param2 <= 0) {
		write(2, "Ha introducido un tiempo de partos incorrecto.\nIntroduzca un tiempo mayor de 0", strlen("Ha introducido un tiempo de partos incorrecto.\nIntroduzca un tiempo mayor de 0"));
		exit(1);
	}

	/* ============= Creacion Recursos ============= */

	ac.sa_handler = intHandler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaddset(&ac.sa_mask, SIGINT);
	if (sigaction(SIGINT, &ac, NULL) == -1) {
		perror("\033[1;31mError en la manejadora SIGINT (CTRL+C)\033[0m\n");
		exit(2);
	}

	/* ============= Creación de la mem compartida ============= */

	mem = -1;
	ptr = (char*) -1;

	// Memoria compartida
	if((mem = shmget(IPC_PRIVATE,sizeof(int)*500,IPC_CREAT|0600))==-1) {
		perror("\033[1;31mError en la creación de la memoria compartida\033[0m\n");
		exit(3);
	}

	// Puntero a la ptr de memoria compartida
	if((ptr = (char *) shmat(mem,NULL,0)) == NULL) {
		perror("\033[1;31mError en la creación del puntero (ptr) de memoria compartida.\033[0m\n");
		exit(3);
	}

	/* ============= Inicialización de los punteros mov ============= */

	for (nProcesos=0; nProcesos<25; nProcesos++) {
		movX = (int*)(ptr+2048+nProcesos*8);
		movY = (int*)(ptr+2048+nProcesos*8+4);
		*movX = -1;
		*movY = -1;
	}


	/* ============= Inicialización de los contadores ============= */
	//Memoria
	r_salvadas = (int*)(ptr+2048+51*sizeof(int));
	//Valor
	*r_salvadas = 0;

	r_nacidas = (int*)(ptr+2048+52*sizeof(int));
	*r_nacidas = 0;

	r_perdidas = (int*)(ptr+2048+53*sizeof(int));
	*r_perdidas = 0;

	/* ============= Creación del lote de semáforos ============= */
	sem=semget(IPC_PRIVATE,7,IPC_CREAT|0600);
	if(sem==-1) {
		perror("\033[1;31mError en la creación del lote de semaforos.");
		exit(4);
	}


/* ============= Inicialización del lote de semáforos ============= */

	// Semáforo 1 (Se encarga de controlar los procesos)
	if(semctl(sem,1,SETVAL, sem1)==-1) {
		perror("\033[1;31mError al inicializar el semáforo 1.");
		exit(4);
	}

	// Semáforo 2 (Se encarga del parto de la 1º rana madre)
	if(semctl(sem,2,SETVAL,sem2)==-1) {
		perror("\033[1;31mError al inicializar el semáforo 2.");
		exit(4);
	}

	// Semáforo 3 (Se encarga del parto de la 2º rana madre)
	if(semctl(sem,3,SETVAL,sem3)==-1) {
		perror("\033[1;31mError al inicializar el semáforo 3.");
		exit(4);
	}

	// Semáforo 4 (Se encarga del parto de la 3º rana madre)
	if(semctl(sem,4,SETVAL,sem4)==-1) {
		perror("\033[1;31mError al inicializar el semáforo 4.");
		exit(4);
	}

	// Semáforo 5 (Se encarga del parto de la 4º rana madre)
	if(semctl(sem,5,SETVAL,sem5)==-1) {
		perror("\033[1;31mError al inicializar el semáforo 5.");
		exit(4);
	}

	// Semáforo 6 (Se encarga de la memoria compartida)
	if(semctl(sem,6,SETVAL,sem6)==-1) {
		perror("\033[1;31mError al inicializar el semáforo 6.");
		exit(4);
	}

	/* ============= Inicio libbatracios.a ============= */

	if(BATR_inicio(param1,sem,lTroncos,lAguas,dirs,param2,ptr)==-1) {
		perror("\033[1;31mError: No se pudo iniciar 'libbatracios.a'.");
		exit(5);
	}

	/* ============= Creación de las ranas y ranitos ============= */

	for(nRana=0; nRana<4; nRana++) {

		switch(fork()) {
		case -1:
			perror("\033[1;31mError en la llamada al sistema 'fork()'.\nError en ranasMadre\033[0m\n");
			exit(6);
		case 0:
			ptr = (char*) shmat(mem,NULL,0);

			//Se llama a la función inthandler
			ac.sa_handler = intHandler;
			sigemptyset(&ac.sa_mask);
			ac.sa_flags = 0;
			sigaddset(&ac.sa_mask, SIGINT);
			if (sigaction(SIGINT, &ac, NULL) == -1) {
				perror("\033[1;31mError en la manejadora del SIGINT (CTRL+C)\033[0m\n");
				exit(2);
			}

			//Se ignora y se evitan procesos zombies
			ac.sa_handler = SIG_IGN;
			sigemptyset(&ac.sa_mask);
			ac.sa_flags = 0;
			sigaddset(&ac.sa_mask, SIGINT);
			if (sigaction(SIGCHLD, &ac, NULL) == -1) {
				perror("\033[1;31mError en la manejadora del SIGCHLD\033[0m\n");
				exit(2);
			}

			rana(nRana);
			exit(0);
		}
	}

	while(noTerminado) {
		for(nTroncos = 0; nTroncos < 7; nTroncos++) {
			sems.sem_num = 6;
			sems.sem_op = -1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				if(errno==EINTR) break;
				perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");
			}

			if(BATR_avance_troncos(nTroncos)==-1) perror("\033[1;31mError al avanzar los troncos.");

			for(nProcesos=0; nProcesos<25; nProcesos++)
			{
				movX = (int*)(ptr+2048+nProcesos*8);
				movY = (int*)(ptr+2048+nProcesos*8+4);

				if((*movY) == 10-nTroncos) {
					if(dirs[nTroncos]==0)
						(*movX)++;
					else
						(*movX)--;
				}
			}
			sems.sem_num =6;
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");

			if(BATR_pausita()==-1) perror("\033[1;31mError en la función: 'BATR_pausita'.\033[0m\n");
		}
	}   //Fin bucle infinito (troncos)
	finPrograma();
}


/* ============= FUNCIONES ============= */

/* ============= Rana ============= */

void rana(int nRana){
	int nProcesos;
	int *movX, *movY; ptr = (char*) shmat(mem,NULL,0); 
	while(noTerminado) {
		BATR_descansar_criar();

		sems.sem_num = nRana+2;
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1)==-1) {
			if(errno==EINTR) break;
			perror("\033[1;31mError semáforo de control de nacimiento de ranaMadre.\033[0m\n");
		}

		sems.sem_num = 1;
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1)==-1) {
			if(errno==EINTR) break;
			perror("\033[1;31mError en el semáforo de procesos máximos.\033[0m\n");
		}

		sems.sem_num = 6;
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			if(errno==EINTR) {
				sems.sem_num = 1;
				sems.sem_op = 1;
				sems.sem_flg = 0;
				if(semop(sem,&sems,1)==-1) perror("\033[1;31mError en el semáforo de procesos máximos.\033[0m\n");
				break;
			} perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");
		}

		for(nProcesos=0; nProcesos<25; nProcesos++) {
			movX=(int *)(ptr+2048+nProcesos*8);
			movY=(int *)(ptr+2048+nProcesos*8+4);

			if(*movY < 0) {

				if(BATR_parto_ranas(nRana,movX,movY)==-1) perror("\033[1;31mError en la función: 'BATR_parto_ranas'.\033[0m\n");
				(*r_nacidas)++;

				switch(fork()) {
				case -1:
					perror("\033[1;31mError en la llamada al sistema 'fork()'.\nError en ranasRanitas\033[0m\n");
					exit(6);

				case 0:
					ac.sa_handler = intHandler;
					sigemptyset(&ac.sa_mask);
					ac.sa_flags = 0;
					sigaddset(&ac.sa_mask, SIGINT);
					if (sigaction(SIGINT, &ac, NULL) == -1) {
						perror("\033[1;31mError en la manejadora del SIGINT (CTRL+C)\033[0m\n");
						exit(2);
					}

					ac.sa_handler = SIG_IGN;
					sigemptyset(&ac.sa_mask);
					ac.sa_flags = 0;
					sigaddset(&ac.sa_mask, SIGINT);
					if (sigaction(SIGCHLD, &ac, NULL) == -1) {
						perror("\033[1;31mError en la manejadora del SIGCHLD\033[0m\n");
						exit(2);
					}
					crias(nProcesos, nRana);
					break;
				}
				ptr = (char*)shmat(mem,NULL,0);
				break;

			}

			if(nProcesos==24) {
				sems.sem_num = 1;
				sems.sem_op = 1;
				sems.sem_flg = 0;
				if(semop(sem,&sems,1)==-1) perror("\033[1;31mError en el semáforo de procesos máximos.\033[0m\n");
			}
		}

		sems.sem_num =6;
		sems.sem_op = 1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");
	}
}

/* ============= Crias ============= */

void crias(int nProcesos, int nRana) {

	int sentido;
	int *movX;
	int *movY;
	int nacimiento=0;

	ptr = (char*) shmat(mem,NULL,0);

	while(noTerminado)
	{
		sems.sem_num =6;
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			if(errno==EINTR)
				break;
			perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");
		}

		movX = (int*)(ptr+2048+nProcesos*8);
		movY = (int*)(ptr+2048+nProcesos*8+4);

		if((*movX) < 0 || (*movX) > 79)
		{
			sems.sem_num = 6;
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");

			(*r_perdidas)++;
			(*movY) = -1;
			(*movX) = -1;
			break;
		}

		if(BATR_puedo_saltar((int)(*movX),(int)(*movY),ARRIBA)==0) sentido = ARRIBA;
		else if(BATR_puedo_saltar((int)(*movX),(int)(*movY),IZQUIERDA)==0) sentido = IZQUIERDA;
		else if(BATR_puedo_saltar((int)(*movX),(int)(*movY),DERECHA)==0) sentido = DERECHA;
		else{
			sems.sem_num = 6;
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");

			BATR_pausa();
			continue;
		}


		if(BATR_avance_rana_ini((int)(*movX),(int)(*movY))==-1) {
			perror("\033[1;31mError en la función: 'BATR_avance_rana_ini'.\033[0m\n");
			exit(5);
		}
		if(BATR_avance_rana((int*)movX,(int*)movY,sentido)==-1) {
			perror("\033[1;31mError en la función: 'BATR_avance_rana'.\033[0m\n");
			exit(5);
		}

		sems.sem_num = 6;
		sems.sem_op = 1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");

		BATR_pausa();

		sems.sem_num = 6;
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			if(errno==EINTR) break;
			perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");
		}

		movX = (int*)(ptr+2048+nProcesos*8);
		movY = (int*)(ptr+2048+nProcesos*8+4);

		if((*movX) < 0 || (*movX) > 79)
		{
			sems.sem_num = 6;
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");

			(*r_perdidas)++;
			(*movY) = -1;
			(*movX) = -1;
			break;
		}


		if(BATR_avance_rana_fin((int)(*movX),(int)(*movY))==-1) {
			perror("\033[1;31mError en la función: 'BATR_avance_rana_fin'.\033[0m\n");
			exit(5);
		}

		if((*movY)==11) {

			sems.sem_num = 6;
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");

			(*r_salvadas)++;
			(*movY) = -1;
			(*movX) = -1;
			break;
		}

		if((*movY) == 1 && nacimiento == 0)
		{
			nacimiento = 1;
			sems.sem_num = nRana+2;
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1)==-1) perror("\033[1;31mError semáforo de control de nacimiento de ranaMadre.\033[0m\n");
		}

		sems.sem_num = 6;
		sems.sem_op = 1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en la memoria compartida (variable:sem)\033[0m\n");
	}

	sems.sem_num = 1;
	sems.sem_op=1;
	sems.sem_flg=0;
	if(semop(sem,&sems,1)==-1) perror("\033[1;31mError en el semáforo de procesos máximos.\033[0m\n");

	exit(0);
}

/* ============= Manejadora ============= */

void intHandler(int a) {
	noTerminado=0;
}

/* ============= finPrograma ============= */

void finPrograma()
{
	int n;

	if (BATR_fin() == -1) {
		perror("\033[1;31mError en la función: 'BATR_fin'.\033[0m\n");
		exit(5);
	}

	for(n=0; n<4; n++) wait(NULL);

	sems.sem_num = 1;
	sems.sem_op = -25;
	sems.sem_flg = 0;
	if(semop(sem,&sems,1) == -1) perror("\033[1;31mError en el semáforo de procesos máximos.\033[0m\n");

	if(BATR_comprobar_estadIsticas(*r_nacidas, *r_salvadas, *r_perdidas)==-1) perror("\033[1;31mError en la función: 'BATR_comprobar_estadIsticas'.\033[0m\n");

	if (semctl(sem, 0, IPC_RMID) == -1) {
		perror("\033[1;31mError al destruir el lote de semáforos.\033[0m\n");
		exit(7);
	} else
		printf("\033[1;100;35mLote de semáforos destruidos.\033[0m\n");

	if (shmctl(mem, IPC_RMID, 0) == -1) {
		perror("\033[1;31mError al destruir la zona de memoria compartida.");
		exit(7);
	} else printf("\033[1;100;35mMemoria compartida destruida.\033[0m\n");

	exit(0);
}
