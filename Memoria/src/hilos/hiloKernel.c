/*
 * hiloKernel.c
 *
 *  Created on: 1/5/2017
 *      Author: utnso
 */

#include "hiloKernel.h"

void* hiloServidorKernel(un_socket socketTemp) {
    /*log_info(logger,"Inicio del hilo Kernel");
    socketKernel = socket_escucha("127.0.0.1", puerto);
    log_debug(logger,"Creacion socket servidor Kernel exitosa");
    listen(socketKernel, 1024);


    socketClienteKernel = aceptar_conexion(socketKernel);
    log_info(logger,"Iniciando Handshake con Kernel\n");
    bool resultado_hand = esperar_handshake(socketClienteKernel,13);
    if(resultado_hand){
        log_debug(logger,"Conexión aceptada del Kernel %d!!", socketClienteKernel);
        enviar(socketClienteKernel,TAMANIO_PAGINA,sizeof(int),&frame_size);
    } else {
        log_error(logger,"Handshake fallo, se aborta conexion");
        exit (EXIT_FAILURE_CUSTOM);
    }*/

    t_paquete * paqueteRecibido;
    void* buffer;
    int pid, paginasRequeridas, pagina, tamanio, offset, tamanioCodigo, paginasCodigo;
    socketClienteKernel = socketTemp;
    while (1) {
    	paqueteRecibido = recibir(socketClienteKernel);
    	if(paqueteRecibido->codigo_operacion==-1){
    		log_error(logger, "KERNEL SE DESCONECTO, ROMPO TODO");
    		pthread_exit(PTHREAD_CANCELED);
    	}

        char * codigoDeOperacion = getCodigoDeOperacion(paqueteRecibido->codigo_operacion);

        log_warning(logger, "Codigo de operacion Memoria-Kernel: %s", codigoDeOperacion);
        switch (paqueteRecibido->codigo_operacion) {
        	case SOLICITAR_BYTES:
				pid = ((t_solicitudBytes*)(paqueteRecibido->data))->pid;
				pagina = ((t_solicitudBytes*)(paqueteRecibido->data))->pagina;
				offset = ((t_solicitudBytes*)(paqueteRecibido->data))->offset;
				tamanio = ((t_solicitudBytes*)(paqueteRecibido->data))->tamanio;

				void* buffer = solicitarBytesDePagina(pid, pagina, offset, tamanio);

				if(buffer == EXIT_FAILURE_CUSTOM) {
					int* fallo = (int*) EXIT_FAILURE_CUSTOM;
					enviar(socketClienteKernel, SOLICITAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "El pedido de KERNEL para SOLICITAR BYTES de PID %i en pagina %i con offset %i y tamanio %i tuvo un fallo",
							pid, pagina, offset, tamanio);

					//free(buffer);

					break;
				}

				void* bufferSerializado = malloc(tamanio);
				memcpy(bufferSerializado, buffer, tamanio);

				enviar(socketClienteKernel, SOLICITAR_BYTES_OK, tamanio, bufferSerializado);
				log_debug(logger, "El pedido de KERNEL para SOLICITAR BYTES de PID %i en pagina %i con offset %i y tamanio %i fue completado correctamente",
						pid, pagina, offset, tamanio);

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
					enviar(socketClienteKernel, ALMACENAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "El pedido de KERNEL para ALMACENAR BYTES de PID %i en pagina %i tuvo un fallo", pid, pagina);

					free(buffer);

					break;
				}

				int* ok = EXIT_SUCCESS_CUSTOM;
				enviar(socketClienteKernel, ALMACENAR_BYTES_OK, sizeof(int), &ok);
				log_debug(logger, "El pedido de KERNEL para ALMACENAR BYTES de PID %i en pagina %i fue completado correctamente", pid, pagina);

				free(buffer);

				break;
            case INICIALIZAR_PROCESO:
            	pid = ((t_inicializar_proceso*) paqueteRecibido->data)->pid;
            	paginasRequeridas = ((t_inicializar_proceso*) paqueteRecibido->data)->paginasTotales;
            	paginasCodigo = ((t_inicializar_proceso*) paqueteRecibido->data)->paginasCodigo;
            	tamanioCodigo = ((t_inicializar_proceso*) paqueteRecibido->data)->sizeCodigo;
            	inicializarProceso(pid, paginasRequeridas);

				log_debug(logger, "El pedido de KERNEL para INICIALIZAR PROCESO de PID %i fue completado correctamente", pid);

                break;
            case FINALIZAR_PROCESO:
                pid = *(int*) paqueteRecibido->data;
            	finalizarProceso(pid);

            	log_debug(logger, "El pedido de KERNEL para FINALIZAR PROCESO de PID %i fue completado correctamente", pid);

            	break;
            case ASIGNAR_PAGINAS:
				paginasRequeridas = ((t_asignarPaginasKernel*)(paqueteRecibido->data))->paginasAsignar;
				pid = ((t_pedidoDePaginasKernel*)(paqueteRecibido->data))->pid;
				int fin = asignarPaginasAProceso(pid, paginasRequeridas);

				if(fin == EXIT_FAILURE_CUSTOM) {
					int* basura = (int*)EXIT_FAILURE_CUSTOM;
					enviar(socketClienteKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), &basura);
					log_error(logger, "El pedido de KERNEL para ASIGNAR PAGINA de PID %i fallo", pid);

					break;
				}

				int* okAsignar = (int*) EXIT_SUCCESS_CUSTOM;
				enviar(socketClienteKernel, ASIGNAR_PAGINAS_OK, sizeof(int), &okAsignar);
				if(paginasRequeridas > 1)
					log_debug(logger, "Se asignaron %i paginas mas al proceso. PID: %i", paginasRequeridas, pid);
				else
					log_debug(logger, "Se asigno una pagina mas al proceso. PID %i", pid);

				log_debug(logger, "El pedido de KERNEL para ASIGNAR PAGINAS de PID %i fue completado correctamente", pid);

				break;
            default:
				log_warning(logger, "Ningun codigo de operacion valido HILO KERNEL");
				break;
        }
    }


}
