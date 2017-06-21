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


	//int pidActual; //para saber que pid esta ejecutando cada hilo

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
				void* bufferAux = solicitarBytesDePagina(pid, pagina, offset, tamanio);
				if(bufferAux != NULL) {
					log_info(logger, "Solicitud de bytes de PID %i OK", pid);
					log_info(logger, "Enviando %i bytes a CPU: PID %i Pagina %i Offset %i ...", tamanio, pid, pagina, offset);
					enviar(socket, SOLICITAR_BYTES_OK, tamanio, bufferAux);
					log_info(logger, "Enviados %i bytes a CPU: PID %i Pagina %i Offset %i ...", tamanio, pid, pagina, offset);
				} else {
					enviar(socket, SOLICITAR_BYTES_FALLO, sizeof(int), -1);
					log_warning(logger, "No se encontraron los bytes solicitados: PID %i Pagina %i Offset %i ...", tamanio, pid, pagina, offset);
				}
				break;
			case ALMACENAR_BYTES:
				pid = ((t_almacenarBytes*)(paqueteRecibido->data))->pid;
				pagina = ((t_almacenarBytes*)(paqueteRecibido->data))->pagina;
				offset = ((t_almacenarBytes*)(paqueteRecibido->data))->offset;
				tamanio = ((t_almacenarBytes*)(paqueteRecibido->data))->tamanio;
				buffer = ((t_almacenarBytes*)(paqueteRecibido->data))->buffer;
				bool exito = almacenarBytesEnPagina(pid, pagina, offset, tamanio, buffer);
				if(exito) {
					int* ok = 1;
					enviar(socket, ALMACENAR_BYTES_OK, sizeof(int), &ok);
					log_info(logger, "Almacenados bytes: PID %i Pagina %i Offset %i Tamanio %i", pid, pagina, tamanio);
				} else {
					int* fallo = -1;
					enviar(socket, ALMACENAR_BYTES_FALLO, sizeof(int), &fallo);
					log_warning(logger, "No se pudieron almacenar bytes: PID %i Pagina %i Offset %i Tamanio %i", pid, pagina, tamanio);
				}
				break;
			default:
				log_error(logger,"Exit por hilo CPU");
				exit(EXIT_FAILURE);
				break;
		}
	}
}



