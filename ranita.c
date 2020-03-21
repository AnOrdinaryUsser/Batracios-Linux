void ranita(int j, int i)
{
    int direccion;
    int *px;
    int *py;
    int parto=0;

    zona= (char *) shmat(memoria,NULL,0);

    while(!terminarAplicacion)
    {
        semaforos.sem_num = 10;  //semaforo de control de memoria compartida
        semaforos.sem_op = -1;
        semaforos.sem_flg = 0;
        if(semop(semaforo,&semaforos,1) == -1){
			if(errno==EINTR)
				break;
            perror("Error semaforo de memoria compartida: ");
        }


        px = (int*)(zona+2048+j*8);
        py = (int*)(zona+2048+j*8+4);


        if((*px) < 0 || (*px) > 79)
        {
            semaforos.sem_num = 10;  //semaforo de control de memoria compartida
            semaforos.sem_op = 1;
            semaforos.sem_flg = 0;
            if(semop(semaforo,&semaforos,1) == -1){
                perror("Error semaforo de memoria compartida: ");
            }

			(*muertas) ++;

            (*py) = -1;
            (*px) = -1;

            break;
        }

        if(BATR_puedo_saltar((int)(*px),(int)(*py),ARRIBA)==0) direccion = ARRIBA;
        else if(BATR_puedo_saltar((int)(*px),(int)(*py),IZQUIERDA)==0) direccion = IZQUIERDA;
        else if(BATR_puedo_saltar((int)(*px),(int)(*py),DERECHA)==0) direccion = DERECHA;
        else{

            semaforos.sem_num = 10;  //semaforo de control de memoria compartida
            semaforos.sem_op = 1;
            semaforos.sem_flg = 0;
            if(semop(semaforo,&semaforos,1) == -1){
                perror("Error semaforo de memoria compartida: ");
            }

            BATR_pausa();
            continue;
        }


        if(BATR_avance_rana_ini((int)(*px),(int)(*py))==-1){
            perror("Error: Avance rana.\n");
            exit(1);
        }
        if(BATR_avance_rana((int*)px,(int*)py,direccion)==-1){
            perror("Error: Avance rana.\n");
            exit(1);
        }

            semaforos.sem_num = 10;  //semaforo de control de memoria compartida
            semaforos.sem_op = 1;
            semaforos.sem_flg = 0;
            if(semop(semaforo,&semaforos,1) == -1){
                perror("Error semaforo de memoria compartida: ");
            }

        BATR_pausa();

		semaforos.sem_num = 10;  //semaforo de control de memoria compartida
        semaforos.sem_op = -1;
        semaforos.sem_flg = 0;
        if(semop(semaforo,&semaforos,1) == -1){
			if(errno==EINTR) break;

            perror("Error semaforo de memoria compartida: ");
        }

        px = (int*)(zona+2048+j*8);
        py = (int*)(zona+2048+j*8+4);


		if((*px) < 0 || (*px) > 79)
        {
            semaforos.sem_num = 10;  //semaforo de control de memoria compartida
            semaforos.sem_op = 1;
            semaforos.sem_flg = 0;
            if(semop(semaforo,&semaforos,1) == -1){
                perror("Error semaforo de memoria compartida: ");
            }
			(*muertas) ++;

            (*py) = -1;
            (*px) = -1;
            break;
        }


        if(BATR_avance_rana_fin((int)(*px),(int)(*py))==-1){
            perror("Error: Final avance rana.\n");
            exit(1);
        }

		if((*py)==11){

            semaforos.sem_num = 10;  //semaforo de control de memoria compartida
            semaforos.sem_op = 1;
            semaforos.sem_flg = 0;
            if(semop(semaforo,&semaforos,1) == -1){
                perror("Error semaforo de memoria compartida: ");
            }
			(*salvadas) ++;

            (*py) = -1;
            (*px) = -1;


            break;
        }

        if((*py) == 1 && parto == 0)
        {
			parto = 1;
            semaforos.sem_num = i+2; //semaforo de control de parto
            semaforos.sem_op = 1;
            semaforos.sem_flg = 0;
            if(semop(semaforo,&semaforos,1)==-1){
                perror("Error semaforo de control de parto");
            }
        }

        semaforos.sem_num = 10;  //semaforo de control de memoria compartida
        semaforos.sem_op = 1;
        semaforos.sem_flg = 0;
        if(semop(semaforo,&semaforos,1) == -1){
            perror("Error semaforo de memoria compartida: ");
        }

    } // Fin bucle infinito

    semaforos.sem_num = 1;  //semaforo de control de procesos maximos
    semaforos.sem_op=1;
    semaforos.sem_flg=0;
    if(semop(semaforo,&semaforos,1)==-1){
        perror("Error semaforo de control de procesos: ");
    }

    exit(0);
} //Fin funcion
