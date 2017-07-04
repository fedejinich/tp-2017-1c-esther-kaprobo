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
	log_debug(logger,"Creacion socket servidor CPU exitosa");
	listen(socketCPU, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketCPU);
		log_info(logger,"Iniciando Handshake con CPU");
		bool resultado_hand = esperar_handshake(socketCliente,15);
		if(resultado_hand){
			log_debug(logger,"Conexión aceptada de la CPU %d!!", socketCliente);
		} else {
			log_info(logger,"Handshake fallo, se aborta conexion");
			exit (EXIT_FAILURE_CUSTOM);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionCPU;
		pthread_create(&conexionCPU, NULL, hiloConexionCPU, (void*)socketClienteTemp);
	}
}


void* hiloConexionCPU(void* socket) {
    t_paquete * paqueteRecibido;


	//int pidActual; //para saber que pid esta ejecutando cada hilo

	while(1) {
		paqueteRecibido = recibir(socket);
		char * codigoDeOperacion = getCodigoDeOperacion(paqueteRecibido->codigo_operacion);

		log_warning(logger, "Codigo de operacion Memoria-CPU: %s", codigoDeOperacion);

		int pid, pagina, offset, tamanio;
		void* buffer;

		switch (paqueteRecibido->codigo_operacion) {
			case SOLICITAR_BYTES:
				pid = ((t_solicitudBytes*)(paqueteRecibido->data))->pid;
				pagina = ((t_solicitudBytes*)(paqueteRecibido->data))->pagina;
				offset = ((t_solicitudBytes*)(paqueteRecibido->data))->offset;
				tamanio = ((t_solicitudBytes*)(paqueteRecibido->data))->tamanio;
				void* bufferAux = solicitarBytesDePagina(pid, pagina, offset, tamanio);
				if(bufferAux == EXIT_FAILURE_CUSTOM) {
					int* fallo = EXIT_FAILURE_CUSTOM;
					enviar(socket, SOLICITAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "No se encontraron los bytes solicitados: PID %i Pagina %i Offset %i ...", tamanio, pid, pagina, offset);
				}
				enviar(socket, SOLICITAR_BYTES_OK, tamanio, bufferAux);
				log_debug(logger, "PID: %i leyo %i bytes de la pagina %i con offset %i y tamanio %i", pid, pagina, offset, tamanio);
				break;
			case ALMACENAR_BYTES:
				pid = ((t_almacenarBytes*)(paqueteRecibido->data))->pid;
				pagina = ((t_almacenarBytes*)(paqueteRecibido->data))->pagina;
				offset = ((t_almacenarBytes*)(paqueteRecibido->data))->offset;
				tamanio = ((t_almacenarBytes*)(paqueteRecibido->data))->tamanio;
				buffer = ((t_almacenarBytes*)(paqueteRecibido->data))->buffer;
				int exito = almacenarBytesEnPagina(pid, pagina, offset, tamanio, buffer);
				if(exito == EXIT_FAILURE_CUSTOM) {
					int* fallo = EXIT_FAILURE_CUSTOM;
					enviar(socket, ALMACENAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "No se pudieron almacenar bytes: PID %i Pagina %i Offset %i Tamanio %i", pid, pagina, tamanio);
				}
				int* ok = EXIT_SUCCESS_CUSTOM;
				enviar(socket, ALMACENAR_BYTES_OK, sizeof(int), &ok);
				log_debug(logger, "Almacenados bytes: PID %i Pagina %i Offset %i Tamanio %i", pid, pagina, tamanio);

				break;
			default:
				log_error(logger,"Exit por hilo CPU");
				log_error(logger, "Tiro un exit(EXIT_FAILURE_CUSTOM) y mato Memoria desde hilo-CPU");
				exit(EXIT_FAILURE_CUSTOM);
				break;
		}
	}
}



