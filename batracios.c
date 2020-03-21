//Revisar funcion ranita
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <signal.h>
#include <time.h>
#include "batracios.h"
#include <errno.h>

/* =================================================================== */
/* =======================VARIABLES GLOBALES========================== */
/* =================================================================== */

// UNION - Necesario para la correcta ejecución en encina
union semun{
  int val;
	struct semid_ds *buf;
};

int sem, mem, noTerminado=1;
char *ptr;
int *r_nacidas, *r_perdidas, *r_salvadas;

struct sembuf sems;

/* ========================================================== */
/* =======================FUNCIONES========================== */
/* ========================================================== */
void INTHANDLER(int retorno);
void finPrograma(void);

/* ===================================================== */
/* =======================MAIN========================== */
/* ===================================================== */
int main (int argc, char *argv[]){

    sigset_t mascara1, mascara2;

  	sigfillset(&mascara1);
  	sigfillset(&mascara2);

  	sigdelset(&mascara2, SIGINT);

	int lTroncos[7]={1,1,1,1,1,1,1};
  int lAguas[7]={1,1,1,1,1,1,1};
  int dirs[7]={1,1,1,1,1,1,1};
  int param1, param2, nTroncos, j, nRana, sentido, nacimiento;
  int *movX, *movY;
  char errorLineaOrdenes[] = "USO: ./batracios VELOCIDAD VELOCIDAD_PARTO\n";

  // Declaración de sems
	union semun sem_0, sem_1, sem_2, sem_3, sem_4, sem_5, sem_6, sem_7, sem_8, sem_9;

	sem_0.val = 25;
	sem_1.val = 1;
	sem_2.val = 1;
	sem_3.val = 1;
	sem_4.val = 1;
  sem_5.val = 1;
  sem_6.val = 1;
  sem_7.val = 1;
  sem_8.val = 1;
  sem_9.val = 1;

  /* ============= Comprobacion de parametros ============= */

  if(argc != 3){
		write(2,errorLineaOrdenes,strlen(errorLineaOrdenes));
		exit(2);
  }

  param1 = atoi(argv[1]);
  param2 = atoi(argv[2]);

  if(param1 < 0 || param1 > 1000){
		write(2, "Ha introducido una velocidad incorrecta.\nPor favor, introduzca una velocidad ente 0 y 1000", strlen("Ha introducido una 			velocidad incorrecta.\nPor favor, introduzca una velocidad ente 0 y 1000"));
		exit(2);
  }

  if (param2 <= 0) {
		write(2, "Ha introducido un tiempo de partos incorrecto.\nIntroduzca un tiempo mayor de 0", strlen("Ha introducido un tiempo de partos 			incorrecto.\nIntroduzca un tiempo mayor de 0"));
  }

  //MANEJADORA SIGINT
  struct sigaction handler;
  handler.sa_handler=INTHANDLER;
  sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaddset(&handler.sa_mask, SIGINT);
  if(sigaction(SIGINT,&handler,NULL)==-1) exit(1);

  	mem = -1;
 		ptr = (char*) -1;

	/* ============= Creación de la mem compartida ============= */


  // mem compartida
  if((mem = shmget(IPC_PRIVATE,sizeof(int)*500,IPC_CREAT|0600))==-1){
		perror("Error al crear la mem compartida.\n");
		exit(3);
  }

  // Puntero a la ptr de mem compartida
  if((ptr = (char *) shmat(mem,NULL,0)) == NULL){
	perror("Error al crear puntero a ptr compartida.\n");
	exit(3);
  }

	/* ============= Creación de el lote de semáforos ============= */

	if((sem = semget(IPC_PRIVATE,11,IPC_CREAT|0666)) == -1){
   	 perror("Error en la creacion del lote de semáforos.\n");
   	 exit(3);
    }

	// Semáforo 0 (Se encarga de controlar los procesos)
	if ((semctl(sem,0,SETVAL,sem_0)) == -1) {
		perror("Inicialización del semáforo [0] incorrecta");
		exit(3);
	}

  // Semáforo 1 (Se encarga del parto de la 1º rana madre)
  if ((semctl(sem,1,SETVAL,sem_1)) == -1) {
		perror("Inicialización del semáforo [1] incorrecta");
		exit(3);
	}

	// Semáforo 2 (Se encarga del parto de la 2º rana madre)
  if ((semctl(sem,2,SETVAL,sem_2)) == -1) {
		perror("Inicialización del semáforo [2] incorrecta");
		exit(3);
	}

	// Semáforo 3 (Se encarga del parto de la 3º rana madre)
  if ((semctl(sem,3,SETVAL,sem_3)) == -1) {
		perror("Inicialización del semáforo [3] incorrecta");
		exit(3);
	}

	// Semáforo 4 (Se encarga del parto de la 4º rana madre)
  if ((semctl(sem,4,SETVAL,sem_4)) == -1) {
		perror("Inicialización del semáforo [4] incorrecta");
		exit(3);
	}

	// Semáforo 5 (Se encarga de la mem compartida)
	if ((semctl(sem,5,SETVAL,sem_5)) == -1) {
		perror("Inicialización del semáforo [5] incorrecta");
		exit(3);
	}

  //Semáforo para controlar que las ranas no se choquen
  if(semctl(sem,6,SETVAL,sem_6)==-1){
      perror("error al inicializar el semaforo 6");
      exit(2);
  }

  //Semáforo para controlar ranas nacidas
  if(semctl(sem,7,SETVAL,sem_7)==-1){
      perror("error al inicializar el semaforo 7");
      exit(2);
  }

  //Semáforo para controlar ranas salvadas
  if(semctl(sem,8,SETVAL,sem_8)==-1){
      perror("error al inicializar el semaforo 8");
      exit(2);
  }

  //Semáforo para controlar ranas muertas
  if(semctl(sem,9,SETVAL,sem_9)==-1){
      perror("error al inicializar el semaforo 9");
      exit(2);
  }
	/* ============= Inicio del programa con BATR_inicio ============= */

  if(BATR_inicio(param1, sem, lTroncos, lAguas, dirs, param2, ptr) != 0){
		write(2,"Error en BATR_inicio\n",strlen("Error en BATR_inicio\n"));
		exit(4);
  }

	/* ============= Creación de las ranas y ranitas ============= */

	for (nRana = 0; nRana < 4; nRana++) {

		switch(fork()) {
			case -1:
				perror("Error en el fork");
				break;

			// Ranas madre
			case 0:
        if((ptr = (char *) shmat(mem,NULL,0)) == NULL){
          perror("Error al crear puntero a ptr compartida.\n");
          exit(3);
        }
				// Manejadora para terminar CTRL+C (PADRE)
  			handler.sa_handler=INTHANDLER;
  			sigemptyset(&handler.sa_mask);
    		handler.sa_flags = 0;
    		sigaddset(&handler.sa_mask, SIGINT);
        if(sigaction(SIGINT,&handler,NULL)==-1) exit(1);

				// Manejadora anti-zombies
				handler.sa_handler = SIG_IGN;
				sigemptyset( &handler.sa_mask);
				handler.sa_flags = 0;
				sigaddset( &handler.sa_mask, SIGINT);
				if (sigaction(SIGCHLD, &handler, NULL) == -1) exit(2);


  			while (noTerminado) {
		  		BATR_descansar_criar();

          // Semáforo de cada rana madre para controlar sus procesos (WAIT)
		  		sems.sem_num = nRana + 1;
		  		sems.sem_op = -1;
		  		sems.sem_flg = 0;
		  		if (semop(sem, &sems, 1) == -1) {
		    		if (errno == EINTR) break;
		    			perror("Error semaforo de control de parto: ");
              exit(777);
		  		}

					// Semáforo que controla el nº de procesos máximos (WAIT)
		  		sems.sem_num = 0;
		  		sems.sem_op = -1;
		  		sems.sem_flg = 0;
		  		if (semop(sem, & sems, 1) == -1) {
		    		if (errno == EINTR) break;
		    			perror("Error semaforo de control de procesos: ");
              exit(888);
		  		}

					// Semáforo que controla la mem compartida (WAIT)
					sems.sem_num = 5;
		  		sems.sem_op = -1;
		  		sems.sem_flg = 0;
          //AQUI DA ERROR TOCHO
		  		if (semop(sem, & sems, 1) == -1) {
		    		if (errno == EINTR) {
		      		sems.sem_num = 0; //sem de control de procesos maximos
		      		sems.sem_op = 1;
		      		sems.sem_flg = 0;
		      		if (semop(sem, & sems, 1) == -1) perror("Error sem de control de procesos: ");
							break;
				   	} perror("Error del semaforo en mem compartida ");
            exit(999);
    			}

      		// Control del nº de procesos
      		for (j = 0; j < 25; j++) {
        		movX = (int * )(ptr + 2048 + j * 8); // 2 bytes, 8 porque son 2 int
        		movY = (int * )(ptr + 2048 + j * 8 + 4);
          	if ( * movY < 0) {

            	if (BATR_parto_ranas(nRana, movX, movY) == -1) {
              	perror("Error: No se realizo parto_ranas en la rana 0");
            	}
            	(*r_nacidas) ++;

            	switch (fork()) {
                case -1:
                  perror("Error en la llamada al sistema fork(). NO ha nacida una ranita");
                  exit(1);
                case 0:
                  // Manejadora CTRL+C
                  handler.sa_handler = INTHANDLER;
                  sigemptyset( & handler.sa_mask);
                  handler.sa_flags = 0;
                  sigaddset( & handler.sa_mask, SIGINT);
                  if (sigaction(SIGINT, & handler, NULL) == -1) exit(1);

                  // Manejara anti-Zombies
                  handler.sa_handler = SIG_IGN;
                  sigemptyset( & handler.sa_mask);
                  handler.sa_flags = 0;
                  sigaddset( & handler.sa_mask, SIGINT);
                  if (sigaction(SIGCHLD, & handler, NULL) == -1) exit(1);

                  // Creación de ranitas (j,nRana)
                  // DEBUG REVISAR DESDE AQUÍ
                  ptr = (char*) shmat(mem, NULL, 0);

                  while (noTerminado) {
                    //sem de control de memoria compartida
                    sems.sem_num = 5;
                    sems.sem_op = -1;
                    sems.sem_flg = 0;
                    if(semop(sem,&sems,1) == -1){
            			       if(errno==EINTR) break;
                         perror("Error sem de memoria compartida: ");
                    }

                    movX = (int*)(ptr+2048+nRana*8);
                    movY = (int*)(ptr+2048+nRana*8+4);

                    // Se comprueba que si se sale de los bytes de la pantalla (horizontal)
                    // Se utiliza para comprobar que no se salen entre que paren y llegan a los troncos
                    if((*movX) < 0 || (*movX) > 79){
                      sems.sem_num = 5;  //sem de control de memoria compartida
                      sems.sem_op = 1;
                      sems.sem_flg = 0;
                      if(semop(sem,&sems,1) == -1) perror("Error sem de memoria compartida: ");
                      // Se cuenta una rana perdida
      			          (*r_perdidas)++;
                      (*movY) = -1;
                      (*movX) = -1;
                      break;
                    }

                    if (BATR_puedo_saltar((int)(*movX),(int)(*movY),ARRIBA)==0) sentido = ARRIBA;
                    else if (BATR_puedo_saltar((int)(*movX),(int)(*movY),DERECHA)==0) sentido = DERECHA;
                    else if (BATR_puedo_saltar((int)(*movX),(int)(*movY),IZQUIERDA)==0) sentido = IZQUIERDA;
                    else{
                      //sem de control de memoria compartida
                      sems.sem_num = 5;
                      sems.sem_op = 1;
                      sems.sem_flg = 0;
                      if(semop(sem,&sems,1) == -1) perror("Error sem de memoria compartida: ");

                      BATR_pausa();
                      continue;
                    }

                    if(BATR_avance_rana_ini((int)(*movX),(int)(*movY))==-1) exit(1);

                    if(BATR_avance_rana((int*)movX,(int*)movY,sentido)==-1) exit(1);

                    sems.sem_num = 5;  //sem de control de memoria compartida
                    sems.sem_op = 1;
                    sems.sem_flg = 0;
                    if(semop(sem,&sems,1) == -1){
                        perror("Error sem de memoria compartida: ");
                    }

                    BATR_pausa();

                    //Sem de control de memoria compartida
            		    sems.sem_num = 5;
                    sems.sem_op = -1;
                    sems.sem_flg = 0;
                    if(semop(sem,&sems,1) == -1){
            			       if(errno==EINTR) break;
                         perror("Error sem de memoria compartida: ");
                    }

                    movX = (int*)(ptr+2048+nRana*8);
                    movY = (int*)(ptr+2048+nRana*8+4);

                    //Este se utiliza para controlar el movimiento (horizontal)
                    //cuando ya están en los troncos
                    if((*movX) < 0 || (*movX) > 79){
                        //sem de control de memoria compartida
                        sems.sem_num = 5;
                        sems.sem_op = 1;
                        sems.sem_flg = 0;
                        if(semop(sem,&sems,1) == -1) perror("Error sem de memoria compartida: ");
            			      (*r_perdidas) ++;
                        (*movY) = -1;
                        (*movX) = -1;
                        break;
                    }

                    if(BATR_avance_rana_fin((int)(*movX),(int)(*movY))==-1) exit(1);

                    //Cuando las ranitas pisan tierra, como Colon en America
                    if((*movY)==11){

                      sems.sem_num = 5;  //semaforo de control de memoria compartida
                      sems.sem_op = 1;
                      sems.sem_flg = 0;
                      if(semop(sem,&sems,1) == -1) perror("Error semaforo de memoria compartida (1): ");
          			      (*r_salvadas) ++;
                      (*movY) = -1;
                      (*movX) = -1;
                      break;
                    }

                    if((*movY) == 1 && nacimiento == 0){
                      nacimiento = 1;
                      //Semaforo de control de parto
                      sems.sem_num = j+1;
                      sems.sem_op = 1;
                      sems.sem_flg = 0;
                      if(semop(sem,&sems,1)==-1) perror("Error semaforo de control de parto");
                    }
                    //Semaforo de control de memoria compartida
                    sems.sem_num = 5;
                    sems.sem_op = 1;
                    sems.sem_flg = 0;
                    if(semop(sem,&sems,1) == -1) perror("Error semaforo de memoria compartida (2): ");
                  } // While infinito

                  sems.sem_num = 0;  //semaforo de control de procesos maximos
                  sems.sem_op=1;
                  sems.sem_flg=0;
                  if(semop(sem,&sems,1)==-1) perror("Error semaforo de control de procesos: ");

                  //break;
                  // DEBUG REVISAR HASTA AQUÍ

                  if(j==24){
          					sems.sem_num = 0;  //semaforo de control de procesos maximos
          					sems.sem_op = 1;
          					sems.sem_flg = 0;
          					if(semop(sem,&sems,1)==-1) perror("Error semaforo de control de procesos: ");
                  }

                  sems.sem_num = 5;  //semaforo de control de memoria compartida
                  sems.sem_op = 1;
                  sems.sem_flg = 0;
                  if(semop(sem,&sems,1) == -1) perror("Error semaforo de memoria compartida (3): ");

                default:
                  break;
              } // switch fork ranitas
            } // If
          } //Fin del for Control de Procesos (25)
        } //Fin del While
      } // Fin del switch
    } // Fin del for de nRana

  //MOVIMIENTO TRONCOS
  while (noTerminado) {
    for(nTroncos=0; nTroncos < 7; nTroncos++){
      //Semaforo de control de memoria compartida
      sems.sem_num = 5;
      sems.sem_op = -1;
      sems.sem_flg = 0;
      if(semop(sem,&sems,1) == -1){
       if(errno==EINTR) break;
       perror("Error semaforo de choque: ");
      }

      if(BATR_avance_troncos(nTroncos)==-1) perror("Error: No pudieron avanzar los troncos de la fila 0");

      for(j=0;j<25;j++) {
         movX = (int*)(ptr+2048+j*8);
         movY = (int*)(ptr+2048+j*8+4);

         if((*movY) == 10-nTroncos){
             if(dirs[nTroncos]==0) (*movX)++;
             else (*movX)--;
         }
      }

      sems.sem_num = 5;  //semaforo de control de memoria compartida
      sems.sem_op = 1;
      sems.sem_flg = 0;
      if(semop(sem,&sems,1)==-1) perror("Error semaforo de choque: ");

      if(BATR_pausita()==-1) perror("Error: No se pudo realizar \"pausita\" de la fila 0");
    }
  } // Fin del bucle  while
  finPrograma();
  return 0;
}

/* ========================================================== */
/* =======================FUNCIONES========================== */
/* ========================================================== */
void INTHANDLER(int retorno){
  noTerminado = 0;
}


void finPrograma(void){
  int i;

  if (BATR_fin() == -1) {
      fprintf(stderr, "Error: No se pudo finalizar BATR_fin\n");
      fflush(stderr);
      exit(1);
  }

  for(i=0;i<4;i++) wait(NULL);

  //Semaforo de control de procesos máximos
  sems.sem_num = 0;
  sems.sem_op = -25;
  sems.sem_flg = 0;
  if(semop(sem,&sems,1) == -1){
    if(errno == EINTR) perror("Se ha producido una interrupcion: ");
    perror("Error semaforo de memoria compartida (4): ");
  }

  if(BATR_comprobar_estadIsticas(*r_nacidas, *r_salvadas, *r_perdidas)==-1) perror("Error: No ha funcionado \"estadisticas\"\n");

  if (semctl(sem, 0, IPC_RMID) == -1) perror("Error: NO se pudo destruir el semaforo.\n");
  else printf("Semaforo destruido.\n");

  if (shmctl(mem, IPC_RMID, 0) == -1) {
      perror("No se ha podido destruir la memoria compartida");
      exit(1);
  }else printf("Memoria compartida destruida\n");

  exit(1);
}
