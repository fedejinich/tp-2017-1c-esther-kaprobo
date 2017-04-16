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

	logger = log_create("memoria.log","Memoria",0,LOG_LEVEL_INFO);

	printf("%s", "\n\n====== INICIO MEMORIA ======\n\n");

	cargarConfiguracion(argv[0]);

	pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);

	pthread_join(servidorConexionesCPU, NULL);
	pthread_join(servidorConexionesKernel, NULL);

	return EXIT_SUCCESS;

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

	log_info(logger,"PUERTO: %i \n", puerto);
	log_info(logger,"CANTIDAD MARCOS: %i \n", marcos);
	log_info(logger,"TAMAÑO MARCO: %i \n", marco_size);
	log_info(logger,"ENTRADAS CACHE DISPONIBLES: %i \n", entradas_cache);
	log_info(logger,"ENTRADAS MAXIMAS POR PROCESO: %i \n", cache_x_proc);
	log_info(logger,"ALGORITMO REEMPLAZO EN CACHE: %s \n", reemplazo_cache);
	log_info(logger,"RETARDO MEMORIA: %i \n", retardo_memoria);
	log_info(logger,"El archivo de configuracion fue cargado con exito\n");
}

void* hiloServidorCPU(void* arg) {
	log_info(logger,"------Hilo CPU------\n");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketCPU = socket_escucha("127.0.0.1", puerto);
	log_info(logger,"Creacion socket servidor CPU exitosa\n\n");
	listen(socketCPU, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketCPU);
		log_info(logger,"Iniciando Handshake con CPU\n");
		bool resultado_hand = esperar_handshake(socketCliente);
		if(resultado_hand){
			log_info(logger,"Conexión aceptada de la CPU %d!!\n", socketCliente);
		} else {
			log_info(logger,"Handshake fallo, se aborta conexion\n");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionCPU;
		pthread_create(&conexionCPU, NULL, hiloConexionCPU, (void*)socketClienteTemp);
	}
}

void* hiloConexionCPU(void* socket) {
	while(1){
		char* buffer = malloc(1000);
		int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			log_warning(logger,"El proceso se desconecto\n");
			return 1;
		}
		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s, de la CPU %d\n", bytesRecibidos, buffer,*(int*)socket);
		free(buffer);
	}
}

void* hiloServidorKernel(void* arg) {
	log_info(logger,"------Hilo Kernel------\n");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketKernel = socket_escucha("127.0.0.1", puerto);
	log_info(logger,"Creacion socket servidor Kernel exitosa\n");
	listen(socketKernel, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketKernel);
		log_info(logger,"Iniciando Handshake con Kernel\n");
		bool resultado_hand = esperar_handshake(socketCliente);
		if(resultado_hand){
			log_info(logger,"Conexión aceptada del Kernel %d!!\n", socketCliente);
		} else {
			log_info(logger,"Handshake fallo, se aborta conexion\n");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionKernel;
		pthread_create(&conexionKernel, NULL, hiloConexionKernel, (void*)socketClienteTemp);
	}
}

void* hiloConexionKernel(void* socket) {
	while(1){
		char* buffer = malloc(1000);
		int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			log_warning(logger,"El proceso se desconecto\n");
			return 1;
		}
		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s, de la Kernel %d\n", bytesRecibidos, buffer,*(int*)socket);
		free(buffer);
	}
}

