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

int main (int argc, char *argv[]){
  int lTroncos[7]={1,1,1,1,1,1,1};
  int lAguas[7]={1,1,1,1,1,1,1};
  int dirs[7]={1,1,1,1,1,1,1};
  int param1;
  int param2;

  int sem;
  int mem = -1;
  char *ptr = (char*) -1;
  char errorLineaOrdenes[] = "USO: ./batracios VEL VEL_PARTO";

  //Memoria compartida
  mem = shmget(IPC_PRIVATE,1,IPC_CREAT|0600);
  ptr = (char *) shmat(mem,0,0);

	//Creación lote 1 semaforo
	if((sem = semget(IPC_PRIVATE,2,IPC_CREAT|0666)) == -1){
		perror("Error en creacion de semaforo");
		return -1;
	}

	//Inicializacion semaforo 0
	if(semctl(sem,0,SETVAL,1) == -1){
    //SETVAL le da 1 pajita
		perror("Error en la inicialización del primer semaforo");
	}

  if(argc != 3){
    write(2,errorLineaOrdenes,strlen(errorLineaOrdenes));
    exit(2);
  }
  //2440
  param1 = atoi(argv[1]);
  param2 = atoi(argv[2]);

  if(param1 < 0 || param1 > 1000){
    write(2, errorLineaOrdenes, strlen(errorLineaOrdenes));
    exit(3);
  }

  if(BATR_inicio(param1, sem, lTroncos, lAguas, dirs, param2, ptr) != 0){
    write(2,"Error en BATR_inicio",strlen("Error en BATR_inicio"));
    exit(4);
  }
  pause();

  //Manejadora de CTRL-C
  //Borra y libera semaforo
  //Manda un SIGINT
  shmdt(ptr);
  semctl(mem, IPC_RMID, 0);
	semctl(sem, 0, IPC_RMID);
  return 0;
}
