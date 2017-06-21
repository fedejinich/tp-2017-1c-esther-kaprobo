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
    t_paquete * paqueteRecibido;


	int pidActual; //para saber que pid esta ejecutando cada hilo

	while(1) {
		paqueteRecibido = recibir(socket);
		char * codigoDeOperacion = getCodigoDeOperacion(paqueteRecibido->codigo_operacion);

		log_info(logger, "Codigo de operacion Memoria-CPU: %s", codigoDeOperacion);

		int pid, pagina, offset, tamanio;
		void* buffer;

		switch (paqueteRecibido->codigo_operacion) {
			case SOLICITAR_BYTES:
				pid = ((t_solicitudBytes*)(paqueteRecibido->data))->pid;
				pagina = ((t_solicitudBytes*)(paqueteRecibido->data))->pagina;
				offset = ((t_solicitudBytes*)(paqueteRecibido->data))->offset;
				tamanio = ((t_solicitudBytes*)(paqueteRecibido->data))->tamanio;
				solicitarBytesDePagina(pid, pagina, offset, tamanio);
				break;
			case ALMACENAR_BYTES:

				break;
			default:
				exit(EXIT_FAILURE);
				break;
		}
	}
}



