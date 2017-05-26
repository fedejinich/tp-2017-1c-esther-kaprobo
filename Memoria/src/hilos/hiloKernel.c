/*
 * hiloKernel.c
 *
 *  Created on: 1/5/2017
 *      Author: utnso
 */

#include "hiloKernel.h"

void* hiloServidorKernel(void* arg) {
	log_info(logger,"Inicio del hilo Kernel");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketKernel = socket_escucha("127.0.0.1", puerto);
	log_info(logger,"Creacion socket servidor Kernel exitosa");
	listen(socketKernel, 1024);

	while(1) {
		socketCliente = aceptar_conexion(socketKernel);
		log_info(logger,"Iniciando Handshake con Kernel\n");
		bool resultado_hand = esperar_handshake(socketCliente,13);
		if(resultado_hand){
			log_info(logger,"Conexión aceptada del Kernel %d!!", socketCliente);
		} else {
			log_info(logger,"Handshake fallo, se aborta conexion");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionKernel;
		pthread_create(&conexionKernel, NULL, hiloConexionKernel, (void*)socketClienteTemp);
	}
}


void* hiloConexionKernel(void* socketKernel) {
	log_info(logger,"Iniciando hilo conexion kernel");

	t_paquete * paquete_nuevo;
	int pid, paginasRequeridas, tamanioCodigo;

	while (1) {
		paquete_nuevo = recibir(socketKernel);

		switch (paquete_nuevo->codigo_operacion) {
			case PEDIDO_DE_PAGINAS:
				paginasRequeridas = ((t_pedidoDePaginas*)(paquete_nuevo->data))->paginasAPedir;
				pid = ((t_pedidoDePaginas*)(paquete_nuevo->data))->pid;

				log_info(logger,"Se piden %i paginas para el proceso %i.",paginasRequeridas, pid);

				if(espacioDisponible(paginasRequeridas, tamanioCodigo)) {
					int i;
					for(i = 1; i <= paginasRequeridas; i++) {
						int frameDisponible = getFrameDisponible();

						t_entradaTablaDePaginas* entrada = malloc(sizeof(t_entradaTablaDePaginas));
						entrada->frame = frameDisponible;
						entrada->pid = pid;
						entrada->pagina = i;

						escribirTablaDePaginas(entrada);
						free(entrada);
					}

					enviar(socketKernel, PEDIDO_DE_PAGINAS_OK, sizeof(int), paginasRequeridas);

					log_info(logger,"Se otorgaron %i paginas al proceso %i.",paginasRequeridas, pid);
				} else {
					enviar(socketKernel, PEDIDO_DE_PAGINAS_FALLO, sizeof(int), -1); //EL TAMANIO Y DATA ESTAN AL PEDO PERO BUEN
					log_warning(logger, "El proceso %i no se pudo inicializar.", pid);
				}

			break;


			default:
				break;
		}
	}


	return 1;
}
