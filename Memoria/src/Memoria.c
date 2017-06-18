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
	grandMalloc();
	inicializarTablaDePaginas();
	inicializarFramePointer();
	//iniciarHilos();
	testFuncionHash();

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
	int tablaDePaginasSize = (frames * sizeof(t_entradaTablaDePaginas));
	tamanioMemoria = (frames * frame_size) + (tablaDePaginasSize); //VER SI MI TABLA DE PAGINAS OCUPA LUGAR DENTRO DE MI MEMORIA
	//SI LLEGA A OCUPAR LUGAR, TENGO QUE RESERVAR LOS N PRIMEROS LUGARES DE MI TABLA DE PAGIAS INVERTIDA
	memoria = malloc(tamanioMemoria);

	if (memoria == NULL) {
		log_error(logger,"No se pudo otorgar la memoria solicitada.");
		exit(EXIT_FAILURE);
	} else
		log_info(logger,"Memoria continua reservada correctamente");

}



void iniciarHilos() {
	log_info(logger, "Inicializando hilos...");

	pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	pthread_create(&consolaMemoria, NULL, hiloConsolaMemoria, NULL);

	pthread_join(servidorConexionesCPU, NULL);
	pthread_join(servidorConexionesKernel, NULL);
	pthread_join(consolaMemoria, NULL);
}

void testFuncionHash() {
	CANTIDAD_DE_MARCOS = 10;
	inicializarOverflow(CANTIDAD_DE_MARCOS);


	int pid=5;
	int pagina=5;

	/* Obtengo el numero de frame candidato con la función hash. */
	int posicion_candidata = calcularPosicion(pid,pagina);

	int posOk = (posicion_candidata == 5);
	printf("Posicion candidata ok? %i\n", posOk);


	int i;
	for(i = 0; i < CANTIDAD_DE_MARCOS; i++) {
		list_destroy(overflow[i]);
	}
	free(overflow);


}
