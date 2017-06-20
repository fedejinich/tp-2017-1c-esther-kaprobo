/*
 * hiloCPU.c
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#include "hiloCPU.h"

void* hiloServidorCPU(void* arg) {
	log_info(logger,"Inicio del hilo CPU");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketCPU = socket_escucha("127.0.0.2", puerto);
	log_info(logger,"Creacion socket servidor CPU exitosa");
	listen(socketCPU, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketCPU);
		log_info(logger,"Iniciando Handshake con CPU");
		bool resultado_hand = esperar_handshake(socketCliente,15);
		if(resultado_hand){
			log_info(logger,"Conexión aceptada de la CPU %d!!", socketCliente);
		} else {
			log_info(logger,"Handshake fallo, se aborta conexion");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionCPU;
		pthread_create(&conexionCPU, NULL, hiloConexionCPU, (void*)socketClienteTemp);
	}
}

void* hiloConexionCPU(void* socket) {
	int pidActual; //para saber que pid esta ejecutando cada hilo



	while(1) {

		char* buffer = malloc(1000);
		int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			log_warning(logger,"El proceso se desconecto");
			return 1;
		}
		buffer[bytesRecibidos] = '\0';
		log_info("Me llegaron %d bytes con %s, de la CPU %d", bytesRecibidos, buffer,*(int*)socket);
		free(buffer);
	}
}

