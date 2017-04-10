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

	iniciarMemoria();
	cargarConfiguracion(argv[0]);
	return EXIT_SUCCESS;

}

void iniciarMemoria(){

	printf("%s", "\n\n====== INICIO MEMORIA ======\n\n");

}

void cargarConfiguracion(char* pathconf){

	t_config* config = config_create(getenv("archivo_configuracion"));
	puerto = config_get_int_value(config, "PUERTO");
	marcos = config_get_int_value(config, "MARCOS");
	marco_size = config_get_int_value(config, "MARCO_SIZE");
	entradas_cache = config_get_int_value(config, "ENTRADAS_CACHE");
	cache_x_proc = config_get_int_value(config, "CACHE_X_PROC");
	reemplazo_cache = config_get_string_value(config, "REEMPLAZO_CACHE");
	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");

	printf("PUERTO: %i \n", puerto);
	printf("CANTIDAD MARCOS: %i \n", marcos);
	printf("TAMAÑO MARCO: %i \n", marco_size);
	printf("ENTRADAS CACHE DISPONIBLES: %i \n", entradas_cache);
	printf("ENTRADAS MAXIMAS POR PROCESO: %i \n", cache_x_proc);
	printf("ALGORITMO REEMPLAZO EN CACHE: %s \n", reemplazo_cache);
	printf("RETARDO MEMORIA: %i \n", retardo_memoria);
	printf("El archivo de configuracion fue cargado con exito\n");
}


