/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "Memoria.h"


int main(int argc, char **argv){
	logger = iniciarLog("memoria.log","Memoria");

	printf("%s", "\n\n====== INICIO MEMORIA ======\n\n");

	//iniciarSeniales();
	cargarConfiguracion();
	grandMalloc(); //aca voy a reservar el bloque de memoria contiuna y crear mi tabla de paginas
	inicializarTablaDePaginas();

	int i;
	for(i=0; i<=21;i++) {
		//if(i == 0 || i == 100 || i == 200 || i == 300 || i == 400 || i == 500)
			printf("Marco: %i, PID: %i, Pagina = %i\n",tablaDePaginas[i].frame,tablaDePaginas[i].pid,tablaDePaginas[i].pagina);
	}

	 printf("hola");
	//iniciarHilos();


	return EXIT_SUCCESS;
}

void cargarConfiguracion(){
	log_info(logger,"Cargando configuracion\n");

	t_config* config = config_create(getenv("archivo_configuracion_memoria"));
	puerto = config_get_int_value(config, "PUERTO");
	log_info(logger,"PUERTO: %i \n", puerto);

	frames = config_get_int_value(config, "MARCOS");
	log_info(logger,"CANTIDAD MARCOS: %i \n", frames);

	frame_size = config_get_int_value(config, "MARCO_SIZE");
	log_info(logger,"TAMAÑO MARCO: %i \n", frame_size);

	entradas_cache = config_get_int_value(config, "ENTRADAS_CACHE");
	log_info(logger,"ENTRADAS CACHE DISPONIBLES: %i \n", entradas_cache);

	cache_x_proc = config_get_int_value(config, "CACHE_X_PROC");
	log_info(logger,"ENTRADAS MAXIMAS POR PROCESO: %i \n", cache_x_proc);

	reemplazo_cache = config_get_string_value(config, "REEMPLAZO_CACHE");
	log_info(logger,"ALGORITMO REEMPLAZO EN CACHE: %s \n", reemplazo_cache);

	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	log_info(logger,"RETARDO MEMORIA: %i \n", retardo_memoria);

	log_info(logger,"El archivo de configuracion fue cargado con exito\n");
}

void grandMalloc() { //aca voy a reservar el bloque de memoria contiuna y crear mi tabla de paginas

	log_info(logger,"Inicio del proceso de reserva de memoria continua\n");

	tamanioMemoria = frames * frame_size;
	memoria = malloc(tamanioMemoria);
	ultimaPosicion = 0;

	if (memoria == NULL) {
		error_show("\x1b[31mNo se pudo otorgar la memoria solicitada.\n\x1b[0m");
		exit(EXIT_FAILURE);
	} else
		log_info(logger,"Memoria continua reservada correctamente\n");

}

void iniciarHilos() {
	//pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	pthread_create(&consolaMemoria, NULL, hiloConsolaMemoria, NULL);

	//pthread_join(servidorConexionesCPU, NULL);
	pthread_join(servidorConexionesKernel, NULL);
}

