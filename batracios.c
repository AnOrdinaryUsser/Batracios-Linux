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

char *ptr;
int mem, sem;
int *r_salvadas, *r_nacidas, *r_perdidas;
int noTerminado=0;
struct sembuf sems;
struct sigaction ac;

union semun {
	int val;
	struct semid_ds *buf;
};

void rana(int i);
void ranita (int j, int madre);
void intHandler(int a);
void finPrograma();

int main (int argc, char*argv[]){

	//int lTroncos[7]={1,1,1,1,1,1,1};
	//int lAguas[7]={1,1,1,1,1,1,1};
	int lTroncos[]={1,2,3,4,3,2,1},lAguas[]={5,4,3,2,3,4,5};
	int dirs[]={1,0,1,0,1,0,1};
	int error, i, j, z;
	int *movX, *movY;
	union semun sem1, sem2, sem3, sem4, sem5, sem6, sem10;

	sem1.val=25;
	sem2.val=1;
	sem3.val=1;
	sem4.val=1;
	sem5.val=1;
	sem6.val=1;
	sem10.val=1;

/*-----------COMPROBACION DE PARAMETROS------------*/

	if(argc>3) {
		fprintf(stderr,"\nError: demasiados parámetros.\n");
		fflush(stderr);
		exit(2);
	}

	if(argc<3) {
		fprintf(stderr,"\nError: debe especificar velocidad y tiempo de descanso entre dos partos.\n");
		fflush(stderr);
		exit(2);
	}

	if(atoi(argv[1])<0 || atoi(argv[1])>1000) {
		fprintf(stderr,"\nMal especificada la velocidad. Debe estar comprendida entre 0 y 1000.\n");
		fflush(stderr);
		exit(2);
	}

	if(atoi(argv[2])<=0) {
		fprintf(stderr,"\nEl tiempo entre dos partos de una rana debe ser mayor que 0.\n");
		fflush(stderr);
		exit(2);
	}


/*-----------FIN COMPROBACION DE PARAMETROS------------*/


	//----------CREACION DE LOS RECURSOS----------

	ac.sa_handler = intHandler;
	sigemptyset(&ac.sa_mask);
	ac.sa_flags = 0;
	sigaddset(&ac.sa_mask, SIGINT);
	if (sigaction(SIGINT, &ac, NULL) == -1) {
		perror("Padre: SIGINT\n");
		exit(1);
	}

	//Memoria compartida

	mem=shmget(IPC_PRIVATE,sizeof(int)*500,IPC_CREAT|0600);
	if(mem == -1) {
		perror("Error: No se pudo crear la mem compartida.");
		exit(2);
	}
	ptr = (char*) shmat(mem,NULL,0);

	if(ptr == NULL) {
		perror("Error: No se pudo asignar el puntero de sentido de mem compartida");
		exit(2);
	}

	for(j=0; j<25; j++)
	{
		movX=(int *)(ptr+2048+j*8);
		movY=(int*)(ptr+2048+j*8+4);
		*movX=-1;
		*movY=-1;
	}

	r_salvadas = (int*)(ptr+2048+51*sizeof(int));
	*r_salvadas = 0;
	r_nacidas = (int*)(ptr+2048+52*sizeof(int));
	*r_nacidas = 0;
	r_perdidas = (int*)(ptr+2048+53*sizeof(int));
	*r_perdidas = 0;

	sem=semget(IPC_PRIVATE,14,IPC_CREAT|0600);
	if(sem==-1) {
		perror("error en la creacion de los sems");
		exit(2);
	}

	error=semctl(sem,1,SETVAL, sem1);   //Semáforo para controlar el número de procesos
	if(error==-1) {
		perror("error al inicializar el sem 1");
		exit(2);
	}

	error=semctl(sem,2,SETVAL,sem2);   //Semáforo para controlar el nacimiento de la rana 1
	if(error==-1) {
		perror("error al inicializar el sem 2");
		exit(2);
	}

	error=semctl(sem,3,SETVAL,sem3);   //Semáforo para controlar el nacimiento de la rana 2
	if(error==-1) {
		perror("error al inicializar el sem 3");
		exit(2);
	}

	error=semctl(sem,4,SETVAL,sem4);   //Semáforo para controlar el nacimiento de la rana 3
	if(error==-1) {
		perror("error al inicializar el sem 4");
		exit(2);
	}

	error=semctl(sem,5,SETVAL,sem5);   //Semáforo para controlar el nacimiento de la rana 4
	if(error==-1) {
		perror("error al inicializar el sem 5");
		exit(2);
	}

	error=semctl(sem,6,SETVAL,sem6);   //Semáforo para controlar que las ranas no se choquen
	if(error==-1) {
		perror("error al inicializar el sem 6");
		exit(2);
	}

	error=semctl(sem,10,SETVAL,sem10); //Semáforo para controlar la mem compartida
	if(error==-1) {
		perror("error al inicializar el sem 10");
		exit(2);
	}



	//---------FIN CREACION DE RECURSOS -------------

	//inicio

	error=BATR_inicio(atoi(argv[1]),sem,lTroncos,lAguas,dirs,atoi(argv[2]),ptr);
	if(error==-1) {
		perror("Error: No se pudo iniciar la librería batracios");
		exit(1);
	}

	//-------------CREACION DE LAS RANAS---------------//

	for(i=0; i<4; i++) {

		switch(fork()) {
		case -1:
			perror("Error: No se pudo crear una ranita");
			exit(1);
		case 0:
			ptr = (char*) shmat(mem,NULL,0);

			ac.sa_handler = intHandler;
			sigemptyset(&ac.sa_mask);
			ac.sa_flags = 0;
			sigaddset(&ac.sa_mask, SIGINT);
			if (sigaction(SIGINT, &ac, NULL) == -1) {
				perror("Padre: SIGINT\n");
				exit(1);
			}

			ac.sa_handler = SIG_IGN;
			sigemptyset(&ac.sa_mask);
			ac.sa_flags = 0;
			sigaddset(&ac.sa_mask, SIGINT);
			if (sigaction(SIGCHLD, &ac, NULL) == -1) {
				perror("Padre: SIGINT\n");
				exit(1);
			}

			rana(i);
			exit(0);
		} //Fin switch

	} //Fin for (for(i = 0; i < 4; i++))

	while(!noTerminado) {

		for(z = 0; z < 7; z++) {

			sems.sem_num = 10;    //sem de control de mem compartida
			sems.sem_op = -1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				if(errno==EINTR) break;
				perror("Error sem de choque: ");
			}

			error=BATR_avance_troncos(z);
			if(error==-1) {
				perror("Error: No pudieron avanzar los troncos de la fila 0");
			}

			for(j=0; j<25; j++)
			{
				movX = (int*)(ptr+2048+j*8);
				movY = (int*)(ptr+2048+j*8+4);

				if((*movY) == 10-z) {
					if(dirs[z]==0) (*movX)++;
					else (*movX)--;
				}
			}

			sems.sem_num = 10;  //sem de control de mem compartida
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				perror("Error sem de choque: ");
			}

			error=BATR_pausita();
			if(error==-1) {
				perror("Error: No se pudo realizar \"pausita\" de la fila 0");
			}
		}
	}   //Fin bucle infinito (troncos)
	finPrograma();
}


// FUNCIONES

void rana(int i){
	int j;
	int *movX, *movY; ptr = (char*) shmat(mem,NULL,0); ptr= (char*) shmat(mem,NULL,0);
	while(!noTerminado) {
		BATR_descansar_criar();

		sems.sem_num = i+2;     //sem de control de nacimiento
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1)==-1) {
			if(errno==EINTR) break;
			perror("Error sem de control de nacimiento: ");
		}

		sems.sem_num = 1;      //sem de control de procesos maximos
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1)==-1) {
			if(errno==EINTR) break;
			perror("Error sem de control de procesos: ");
		}

		//sem de control de mem compartida (506)
		sems.sem_num = 10;
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			if(errno==EINTR) {
				sems.sem_num = 1;      //sem de control de procesos maximos
				sems.sem_op = 1;
				sems.sem_flg = 0;
				if(semop(sem,&sems,1)==-1) perror("Error sem de control de procesos: ");
				break;
			} perror("Error sem de mem compartida (0): ");
		}

		//Por qué no un 20? DEBUG
		for(j=0; j<25; j++) {
			//Se mueve acordes a la rana madre
			//2048?? DEBUG
			movX=(int *)(ptr+2048+j*8);
			movY=(int *)(ptr+2048+j*8+4);

			if(*movY < 0) {

				if(BATR_parto_ranas(i,movX,movY)==-1) perror("Error: No se realizo parto_ranas en la rana 0");
				(*r_nacidas)++;

				switch(fork()) {
				case -1:
					perror("Error: No se pudo crear una ranita");
					exit(1);

				case 0:
					ac.sa_handler = intHandler;
					sigemptyset(&ac.sa_mask);
					ac.sa_flags = 0;
					sigaddset(&ac.sa_mask, SIGINT);
					if (sigaction(SIGINT, &ac, NULL) == -1) {
						perror("Padre: SIGINT\n");
						exit(1);
					}

					ac.sa_handler = SIG_IGN;
					sigemptyset(&ac.sa_mask);
					ac.sa_flags = 0;
					sigaddset(&ac.sa_mask, SIGINT);
					if (sigaction(SIGCHLD, &ac, NULL) == -1) {
						perror("Padre: SIGINT\n");
						exit(1);
					}
					ranita(j, i);
					break;
				}  //FIN DEL SWITCH
				ptr= (char*) shmat(mem,NULL,0);
				break;

			}

			if(j==24) {
				sems.sem_num = 1;      //sem de control de procesos maximos
				sems.sem_op = 1;
				sems.sem_flg = 0;
				if(semop(sem,&sems,1)==-1) perror("Error sem de control de procesos (9): ");
			}
		}

		//Sem de control de mem compartida (269)
		sems.sem_num = 10;
		sems.sem_op = 1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			perror("Error sem de mem compartida (10): ");
		}
	}
}

void ranita(int i, int madre) {

	int sentido;
	int *movX;
	int *movY;
	int nacimiento=0;

	ptr= (char*) shmat(mem,NULL,0);

	while(!noTerminado)
	{
		sems.sem_num = 10;      //sem de control de mem compartida
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			if(errno==EINTR)
				break;
			perror("Error sem de mem compartida: ");
		}


		movX = (int*)(ptr+2048+i*8);
		movY = (int*)(ptr+2048+i*8+4);


		if((*movX) < 0 || (*movX) > 79)
		{
			sems.sem_num = 10;        //sem de control de mem compartida
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				perror("Error sem de mem compartida: ");
			}

			(*r_perdidas)++;

			(*movY) = -1;
			(*movX) = -1;

			break;
		}

		if(BATR_puedo_saltar((int)(*movX),(int)(*movY),ARRIBA)==0) sentido = ARRIBA;
		else if(BATR_puedo_saltar((int)(*movX),(int)(*movY),IZQUIERDA)==0) sentido = IZQUIERDA;
		else if(BATR_puedo_saltar((int)(*movX),(int)(*movY),DERECHA)==0) sentido = DERECHA;
		else{

			sems.sem_num = 10;        //sem de control de mem compartida
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				perror("Error sem de mem compartida: ");
			}

			BATR_pausa();
			continue;
		}


		if(BATR_avance_rana_ini((int)(*movX),(int)(*movY))==-1) {
			perror("Error: Avance rana.\n");
			exit(1);
		}
		if(BATR_avance_rana((int*)movX,(int*)movY,sentido)==-1) {
			perror("Error: Avance rana.\n");
			exit(1);
		}

		sems.sem_num = 10;          //sem de control de mem compartida
		sems.sem_op = 1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			perror("Error sem de mem compartida: ");
		}

		BATR_pausa();

		sems.sem_num = 10;  //sem de control de mem compartida
		sems.sem_op = -1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			if(errno==EINTR) break;

			perror("Error sem de mem compartida: ");
		}

		movX = (int*)(ptr+2048+i*8);
		movY = (int*)(ptr+2048+i*8+4);


		if((*movX) < 0 || (*movX) > 79)
		{
			sems.sem_num = 10;        //sem de control de mem compartida
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				perror("Error sem de mem compartida: ");
			}
			(*r_perdidas)++;

			(*movY) = -1;
			(*movX) = -1;
			break;
		}


		if(BATR_avance_rana_fin((int)(*movX),(int)(*movY))==-1) {
			perror("Error: Final avance rana.\n");
			exit(1);
		}

		if((*movY)==11) {

			sems.sem_num = 10;        //sem de control de mem compartida
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1) == -1) {
				perror("Error sem de mem compartida: ");
			}
			(*r_salvadas)++;

			(*movY) = -1;
			(*movX) = -1;


			break;
		}

		if((*movY) == 1 && nacimiento == 0)
		{
			nacimiento = 1;
			sems.sem_num = madre+2;       //sem de control de nacimiento
			sems.sem_op = 1;
			sems.sem_flg = 0;
			if(semop(sem,&sems,1)==-1) {
				perror("Error sem de control de nacimiento");
			}
		}

		sems.sem_num = 10;      //sem de control de mem compartida
		sems.sem_op = 1;
		sems.sem_flg = 0;
		if(semop(sem,&sems,1) == -1) {
			perror("Error sem de mem compartida: ");
		}

	}   // Fin bucle infinito

	sems.sem_num = 1;    //sem de control de procesos maximos
	sems.sem_op=1;
	sems.sem_flg=0;
	if(semop(sem,&sems,1)==-1) {
		perror("Error sem de control de procesos: ");
	}

	exit(0);
}    //Fin funcion */


void intHandler(int a) {
	noTerminado=1;
}

void finPrograma()
{
	int n;


	if (BATR_fin() == -1) {
		fprintf(stderr, "Error: No se pudo finalizar\n");
		fflush(stderr);
		exit(1);
	}

	for(n=0; n<4; n++)
		wait(NULL);


	sems.sem_num = 1;    //sem de control de mem compartida
	sems.sem_op = -25;
	sems.sem_flg = 0;
	if(semop(sem,&sems,1) == -1)
		perror("Error sem de mem compartida (777): ");


	sems.sem_num = 0;    //sem de control de mem compartida
	sems.sem_op = 1;
	sems.sem_flg = 0;
	if(semop(sem,&sems,1) == -1) {
		perror("Error sem de mem compartida (888): ");
	}

	if(BATR_comprobar_estadIsticas(*r_nacidas, *r_salvadas, *r_perdidas)==-1) {
		perror("Error: No ha funcionado \"estadisticas\"\n");
	}

	if (semctl(sem, 0, IPC_RMID) == -1) {
		perror("Error: NO se pudo destruir el sem.\n");

	}else{
		printf("Semaforo destruido.\n");
	}

	if (shmctl(mem, IPC_RMID, 0) == -1) {
		perror("Error: NO se pudo destruir la mem compartida");
		exit(1);
	}else{
		printf("Memoria compartida destruida\n");
	}

	exit(1);
}
