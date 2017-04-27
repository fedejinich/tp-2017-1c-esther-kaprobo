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
	grandMalloc();
	//crearEstructurasAdministrativas();
	esperarAlKernel();
	iniciarServidor();



	return EXIT_SUCCESS;

}

void esperarAlKernel() {

}


void cargarConfiguracion(){

	t_config* config = config_create(getenv("archivo_configuracion_memoria"));
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

void grandMalloc() {
	memoria = malloc(marcos * marco_size);

	tablaDePaginas = list_create();

	if (memoria == NULL) {
		error_show("\x1b[31mNo se pudo otorgar la memoria solicitada.\n\x1b[0m");
		exit(EXIT_FAILURE);
	}
}

void iniciarServidor() {
	pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);

	pthread_join(servidorConexionesCPU, NULL);
	pthread_join(servidorConexionesKernel, NULL);
}

void* hiloServidorCPU(void* arg) {
	log_info(logger,"------Hilo CPU------\n");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketCPU = socket_escucha("127.0.0.2", puerto);
	log_info(logger,"Creacion socket servidor CPU exitosa\n\n");
	listen(socketCPU, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketCPU);
		log_info(logger,"Iniciando Handshake con CPU\n");
		bool resultado_hand = esperar_handshake(socketCliente,15);
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
		bool resultado_hand = esperar_handshake(socketCliente,13);
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

/*void* hiloConexionKernel(void* socketKernel) {
	t_paquete * paquete_nuevo;

	int pid, paginasRequeridas, tamanioCodigo;
	char* codigo;

	/*while (1) {

		paquete_nuevo = recibir(socketKernel);
		switch (paquete_nuevo->codigo_operacion) {
			case INICIALIZAR:
				memcpy(&pid, paquete_nuevo->data, sizeof(int));
				memcpy(&paginasRequeridas, paquete_nuevo->data + sizeof(int),sizeof(int));
				memcpy(&tamanioCodigo, paquete_nuevo->data + sizeof(int) * 2,sizeof(int));
				memcpy(codigo, paquete_nuevo->data + sizeof(int) * 3,tamanioCodigo);

				log_info(logger,"Llega un nuevo proceso, las paginas requeridas son %d.\n",paginasRequeridas);

				codigo = malloc(tamanioCodigo);

				if (puede_iniciar_proceso(pid, paginasRequeridas, codigo)) {
					inicializar_programa(pid, paginasRequeridas);
					log_info(logger,"Se pudo inicializar el proceso con el pid %d.\n",pid);
					enviar(socketKernel, EXITO, sizeof(int), &pid);
				} else {
					log_info(logger,"No se pudo inicializar el proceso con el pid %d.\n",pid);
					enviar(socketKernel, FRACASO, sizeof(int), &pid);
				}
				free(codigo);
			break;
			default:
				break;
		}

	}
}*/

void* hiloConexionKernel(void* socket) {
	while(1){

//		AHORA TENGO QUE ESPERAR A RECIBIR UN ARCHIVO, CON SU RESPECTIVO RETARDO
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

