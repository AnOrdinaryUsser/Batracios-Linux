#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <time.h>
#include "batracios.h"

/* =================================================================== */
/* =======================VARIABLES GLOBALES========================== */
/* =================================================================== */

/* Necesario para ejecutar en encina
union semun{
	int val;
	struct semid_ds *buf;
};
*/
int sem,mem;
char *ptr;
//struct sembuf sems;


/* ========================================================== */
/* =======================FUNCIONES========================== */
/* ========================================================== */
void INTHANDLER(int retorno);


/* ===================================================== */
/* =======================MAIN========================== */
/* ===================================================== */
int main (int argc, char *argv[]){
  int lTroncos[7]={1,1,1,1,1,1,1};
  int lAguas[7]={1,1,1,1,1,1,1};
  int dirs[7]={1,1,1,1,1,1,1};
  int param1, param2;
  char errorLineaOrdenes[] = "USO: ./batracios VEL VEL_PARTO\n";

  //MANEJADORA SIGINT
  struct sigaction sigint;
  sigint.sa_handler=INTHANDLER;
  sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = 0;
	sigaddset(&sigint.sa_mask, SIGINT);
  if(sigaction(SIGINT,&sigint,NULL)==-1) exit(1);

  mem = -1;
  ptr = (char*) -1;

  /* Comprobacion de parametros */
  if(argc != 3){
    write(2,errorLineaOrdenes,strlen(errorLineaOrdenes));
    exit(2);
  }

  param1 = atoi(argv[1]);
  param2 = atoi(argv[2]);

  if(param1 < 0 || param1 > 1000){
    write(2, "Ha introducido una velocidad incorrecta.\nPor favor, introduzca una velocidad ente 0 y 1000", strlen("Ha introducido una velocidad incorrecta.\nPor favor, introduzca una velocidad ente 0 y 1000"));
    exit(2);
  }

  if (param2 <= 0) {
    write(2, "Ha introducido un tiempo de partos incorrecto.\nIntroduzca un tiempo mayor de 0", strlen("Ha introducido un tiempo de partos incorrecto.\nIntroduzca un tiempo mayor de 0"));
  }

  //Memoria compartida
  if((mem = shmget(IPC_PRIVATE,sizeof(int)*500,IPC_CREAT|0600))==-1){
    perror("Error al crear la memoria compartida.\n");
    exit(3);
  }
  if((ptr = (char *) shmat(mem,NULL,0))== NULL){
    perror("Error al crear puntero a zona compartida.\n");
    exit(3);
  }

	//Creación lote 1 semaforo
	if((sem = semget(IPC_PRIVATE,1,IPC_CREAT|0666)) == -1){
		perror("Error en creacion de semaforo.\n");
		exit(3);
	}

	//Inicializacion semaforo 0
	if(semctl(sem,0,SETVAL,1) == -1){
    //SETVAL le da 1 pajita
		perror("Error en la inicialización del primer semaforo.\n");
    exit(3);
	}

  if(BATR_inicio(param1, sem, lTroncos, lAguas, dirs, param2, ptr) != 0){
    write(2,"Error en BATR_inicio\n",strlen("Error en BATR_inicio\n"));
    exit(4);
  }

  return 0;
}

/* ========================================================== */
/* =======================FUNCIONES========================== */
/* ========================================================== */
void INTHANDLER(int retorno){

  // Llamada a BATR_fin
  if (BATR_fin() == -1) {
          fprintf(stderr, "Finalización incorrecta del programa\n");
          fflush(stderr);
          exit(1);
  }

  //Borra y libera semaforo
  shmdt(ptr);

  if(semctl(sem, 0, IPC_RMID)==-1){
    perror("Error al destruir un semaforo\n");
  } else write(2,"Liberando semaforos.\n",strlen("Liberando semaforos.\n"));

  if(shmctl(mem, IPC_RMID, 0)==-1){
    perror("Error al liberar memora compartida.\n");
  } else write(2,"Liberando memoria compartida.\n",strlen("Liberando memoria compartida.\n"));

  exit(1);
}
