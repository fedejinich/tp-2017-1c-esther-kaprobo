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



	printf("%s", "\n====== INICIO MEMORIA ======\n\n");

	//iniciarSeniales();
	cargarConfiguracion();
	grandMalloc(); //aca voy a reservar el bloque de memoria contiuna y crear mi tabla de paginas
	inicializarTablaDePaginas();
	//iniciarHilos();

	int entrada = 21;
	int entrada2 = 0;
	int entrada3 = 451;
	int entrada4 = 84;
	int entrada5 = 500;
	int entrada6 = 235;
	printf("Entrada %i, Frame = %i, Ubicacion dentro del frame = %i\n",entrada,numeroDeFrameBy(entrada),numeroDeEntradaEnFrameBy(entrada));
	printf("Entrada %i, Frame = %i, Ubicacion dentro del frame = %i\n",entrada2,numeroDeFrameBy(entrada2),numeroDeEntradaEnFrameBy(entrada2));
	printf("Entrada %i, Frame = %i, Ubicacion dentro del frame = %i\n",entrada3,numeroDeFrameBy(entrada3),numeroDeEntradaEnFrameBy(entrada3));
	printf("Entrada %i, Frame = %i, Ubicacion dentro del frame = %i\n",entrada4,numeroDeFrameBy(entrada4),numeroDeEntradaEnFrameBy(entrada4));
	printf("Entrada %i, Frame = %i, Ubicacion dentro del frame = %i\n",entrada5,numeroDeFrameBy(entrada5),numeroDeEntradaEnFrameBy(entrada5));
	printf("Entrada %i, Frame = %i, Ubicacion dentro del frame = %i\n",entrada6,numeroDeFrameBy(entrada6),numeroDeEntradaEnFrameBy(entrada6));




	escribir_frame(31,0,4,"hola");
	char* buffer = malloc(4);
	memcpy(buffer,&framePointer[31],4);
	printf("\nasd: %s\n",buffer);

	return EXIT_SUCCESS;
}

void cargarConfiguracion(){
	log_info(logger,"Cargando configuracion");

	t_config* config = config_create(getenv("archivo_configuracion_memoria"));
	puerto = config_get_int_value(config, "PUERTO");
	log_info(logger,"PUERTO: %i ", puerto);

	frames = config_get_int_value(config, "MARCOS");
	log_info(logger,"CANTIDAD MARCOS: %i ", frames);

	frame_size = config_get_int_value(config, "MARCO_SIZE");
	log_info(logger,"TAMAÑO MARCO: %i ", frame_size);

	entradas_cache = config_get_int_value(config, "ENTRADAS_CACHE");
	log_info(logger,"ENTRADAS CACHE DISPONIBLES: %i ", entradas_cache);

	cache_x_proc = config_get_int_value(config, "CACHE_X_PROC");
	log_info(logger,"ENTRADAS MAXIMAS POR PROCESO: %i ", cache_x_proc);

	reemplazo_cache = config_get_string_value(config, "REEMPLAZO_CACHE");
	log_info(logger,"ALGORITMO REEMPLAZO EN CACHE: %s ", reemplazo_cache);

	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	log_info(logger,"RETARDO MEMORIA: %i ", retardo_memoria);

	log_info(logger,"El archivo de configuracion fue cargado con exito");
}

void grandMalloc() { //aca voy a reservar el bloque de memoria contiuna y crear mi tabla de paginas`

	log_info(logger,"Reservando bloque de memoria contigua...");

	tamanioMemoria = frames * frame_size;
	memoria = malloc(tamanioMemoria);
	framePointer = &memoria[0]; //Inicializo el framePointer al principio de mi memoria

	if (memoria == NULL) {
		log_error(logger,"No se pudo otorgar la memoria solicitada.");
		exit(EXIT_FAILURE);
	} else
		log_info(logger,"Memoria continua reservada correctamente");

}

void iniciarHilos() {
	//pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	pthread_create(&consolaMemoria, NULL, hiloConsolaMemoria, NULL);

	//pthread_join(servidorConexionesCPU, NULL);
	pthread_join(servidorConexionesKernel, NULL);
}

