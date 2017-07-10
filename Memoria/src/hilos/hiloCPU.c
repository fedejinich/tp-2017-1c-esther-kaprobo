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
		//pthread_create(&conexionCPU, NULL, hiloConexionCPU, (void*)socketClienteTemp);
		pthread_create(&conexionCPU, NULL, hiloConexionCPU, socketCliente);
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

				buffer = solicitarBytesDePagina(pid, pagina, offset, tamanio);

				if(buffer == EXIT_FAILURE_CUSTOM) {
					int* fallo = EXIT_FAILURE_CUSTOM;
					enviar(socket, SOLICITAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "El pedido de CPU para SOLICITAR BYTES tuvo un fallo");

					free(buffer);

					break;
				}

				log_warning(logger, "Buffer Memoria - HiloCPU: %s", buffer);

				enviar(socket, SOLICITAR_BYTES_OK, tamanio, buffer);

				log_debug(logger, "El pedido de CPU para SOLICITAR BYTES fue completado correctamente");

				free(buffer);

				break;
			case ALMACENAR_BYTES:
				memcpy(&pid, paqueteRecibido->data, sizeof(int));
				memcpy(&pagina, paqueteRecibido->data + sizeof(int), sizeof(int));
				memcpy(&offset, paqueteRecibido->data + sizeof(int) * 2, sizeof(int));
				memcpy(&tamanio, paqueteRecibido->data + sizeof(int) * 3, sizeof(int));

				buffer = malloc(tamanio);
				memcpy(buffer, paqueteRecibido->data + sizeof(int) * 4, tamanio);

				int exito = almacenarBytesEnPagina(pid, pagina, offset, tamanio, buffer);

				if(exito == EXIT_FAILURE_CUSTOM) {
					int* fallo = EXIT_FAILURE_CUSTOM;
					enviar(socket, ALMACENAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "El pedido de CPU para ALMACENAR BYTES tuvo un fallo");

					free(buffer);

					break;
				}

				log_warning(logger, "Exito %i", exito);


				int* ok = EXIT_SUCCESS_CUSTOM;
				enviar(socket, ALMACENAR_BYTES_OK, sizeof(int), &ok);
				log_debug(logger, "El pedido de CPU para ALMACENAR BYTES fue completado correctamente");

				free(buffer);

				break;
			case -1:
				log_warning(logger, "Se desconecto una CPU");
				break;
			default:
				log_error(logger,"Exit por hilo CPU");
				log_error(logger, "Tiro un exit(EXIT_FAILURE_CUSTOM) y mato Memoria desde hilo-CPU");
				exit(EXIT_FAILURE_CUSTOM);
				break;
		}
	}
}



