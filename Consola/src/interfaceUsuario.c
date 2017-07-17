/*
 * interfaceUsuario.c
 *
 *  Created on: 10/7/2017
 *      Author: utnso
 */


#include "interfaceUsuario.h"


void interface(){
	while(1){
			pthread_mutex_lock(&mutexEjecuta);
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			scanf("%i",&opcion);

			//opciones para consola
			switch(opcion) {
			case 1 :
				//Inicia nuevo programa AnSISOP, va a enviar el path del script
				pthread_mutex_lock(&mutexEjecuta);
				iniciarPrograma();
				break;
			case 2:
				//Finaliza la ejecucion de un programa, lleva el PID correspondiente
				pthread_mutex_lock(&mutexEjecuta);
				finalizarPrograma();
				pthread_mutex_unlock(&mutexEjecuta);
				break;
			case 3:
				//Desconecta todos los hilos de conexion con kernel, abortando todos los programas en ejecucion
				pthread_mutex_lock(&mutexEjecuta);
				desconectarConsola();
				pthread_mutex_unlock(&mutexEjecuta);
				break;
			case 4:
				//Elimina todos los mensajes de la pantalla
				pthread_mutex_lock(&mutexEjecuta);
				limpiarMensajes();
				pthread_mutex_unlock(&mutexEjecuta);
				break;
			default:
				printf("Opcion invalida, vuelva a intentar\n\n");
				break;
			}

		}
}


void iniciarPrograma(){

	printf("Iniciar Programa\n\n");
	pthread_create(&threadNewProgram, NULL, hiloNuevoPrograma, NULL);
	//pthread_join(threadNewProgram, NULL);
	return;
}

void finalizarPrograma(){
	int n;
	signed int soc;
	printf("Finalizar Programa\n\n");
	printf("Ingrese el PID a finalizar: \n");
	scanf("%i",&n);
	soc = matriz[n];

	log_debug(logger,"Se finalizara PID: %d, en socket:%d ", n, soc);


	if(soc > 0){
		enviar(soc, FINALIZAR_PROGRAMA_DESDE_CONSOLA, sizeof(int), &n);

		close(soc);
		matriz[n] = 0;

	}

	else
		printf("El pid %d ya no se encuentra conectado al Kernel\n", n);
	if(pthread_cancel(matrizHilos[n]) == 0){
		log_debug(logger,"Hilo finalizado");
	}
	else{
		log_error(logger,"error al finalizar Hilo");
	}

	return;
}

void desconectarConsola(){
	int i;
	signed int soc;
	log_info(logger, "Desconectar Consola\n\n");
	for( i= 0; i< MAXPID; i++){

		soc = matriz[i];
		if (soc > 0){
			int algo;
			enviar(soc, FINALIZAR_PROGRAMA_DESDE_CONSOLA, sizeof(int), algo);
			close(soc);
			printf("Se cierra pid: %d \n", i);
			matriz[i] = 0;
			if(pthread_cancel(matrizHilos[i]) == 0){
				printf("Hilo finalizado\n");

				}
			else{
				printf("error al finalizar Hilo\n");
				}
		}
	}

	return;
}

void limpiarMensajes(){

	printf("Limpiar Mensajes\n\n");
	system("clear");

	return;
}


void mostrarMenu(){

	printf("\nIngrese la opcion deseada:\n");
	printf("OPCION 1 - INICIAR PROGRAMA\n");
	printf("OPCION 2 - FINALIZAR PROGRAMA\n");
	printf("OPCION 3 - DESCONECTAR CONSOLA\n");
	printf("OPCION 4 - LIMPIAR MENSAJES\n");
	printf("Su Opcion:\n");

}
