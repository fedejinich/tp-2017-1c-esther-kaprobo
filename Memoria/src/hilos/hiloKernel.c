/*
 * hiloKernel.c
 *
 *  Created on: 1/5/2017
 *      Author: utnso
 */

#include "hiloKernel.h"

void* hiloServidorKernel(void* arg) {
    log_info(logger,"Inicio del hilo Kernel");
    int servidorSocket ;
    int *socketClienteTemp;
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
    }

    t_paquete * paqueteRecibido;
    int pid, paginasRequeridas, pagina, tamanio, offset, tamanioCodigo, paginasCodigo;
    char* codigo;
    void* datas;

    t_asignarPaginasKernel* data;
    void* buffer; char* string;



    while (1) {
    	paqueteRecibido = recibir(socketClienteKernel);

        char * codigoDeOperacion = getCodigoDeOperacion(paqueteRecibido->codigo_operacion);

        log_warning(logger, "Codigo de operacion Memoria-Kernel: %s", codigoDeOperacion);
        switch (paqueteRecibido->codigo_operacion) {
            case INICIALIZAR_PROCESO:
            	pid = ((t_inicializar_proceso*) paqueteRecibido->data)->pid;
            	paginasRequeridas = ((t_inicializar_proceso*) paqueteRecibido->data)->paginasTotales;
            	paginasCodigo = ((t_inicializar_proceso*) paqueteRecibido->data)->paginasCodigo;
            	tamanioCodigo = ((t_inicializar_proceso*) paqueteRecibido->data)->sizeCodigo;
            	inicializarProceso(pid, paginasRequeridas);

                break;
            case ASIGNAR_PAGINAS:
            	paginasRequeridas = ((t_asignarPaginasKernel*)(paqueteRecibido->data))->paginasAsignar;
                pid = ((t_pedidoDePaginasKernel*)(paqueteRecibido->data))->pid;
                asignarPaginasAProceso(pid, paginasRequeridas);

                break;
            case FINALIZAR_PROCESO:
                pid = (int)paqueteRecibido->data;
            	finalizarProceso(pid);

            	break;
            case SOLICITAR_BYTES:
            	pid = ((t_solicitudBytes*)(paqueteRecibido->data))->pid;
            	pagina = ((t_solicitudBytes*)(paqueteRecibido->data))->pagina;
            	offset = ((t_solicitudBytes*)(paqueteRecibido->data))->offset;
            	tamanio = ((t_solicitudBytes*)(paqueteRecibido->data))->tamanio;

            	void* bufferAux = solicitarBytesDePagina(pid, pagina, offset, tamanio);
				if(bufferAux == EXIT_FAILURE_CUSTOM) {
					int* fallo = EXIT_FAILURE_CUSTOM;
					enviar(socketClienteKernel, SOLICITAR_BYTES_FALLO, sizeof(int), &fallo);
					log_error(logger, "No se encontraron los bytes solicitados: PID %i Pagina %i Offset %i ...", tamanio, pid, pagina, offset);
				}
				enviar(socketClienteKernel, SOLICITAR_BYTES_OK, tamanio, bufferAux);
				log_debug(logger, "PID: %i leyo %i bytes de la pagina %i con offset %i y tamanio %i", pid, pagina, offset, tamanio);

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
					log_error(logger, "No se pudieron almacenar bytes: PID %i Pagina %i Offset %i Tamanio %i", pid, pagina, tamanio);
				}

				int* ok = EXIT_SUCCESS_CUSTOM;
				enviar(socketClienteKernel, ALMACENAR_BYTES_OK, sizeof(int), &ok);
				log_debug(logger, "Almacenados bytes: PID %i Pagina %i Offset %i Tamanio %i", pid, pagina, tamanio);
				free(buffer);

            	break;
            default:
				log_error(logger, "Exit por hilo Kernel");
				log_error(logger, "Tiro un exit(EXIT_FAILURE_CUSTOM) desde hilo-Kernel");
            	exit(EXIT_FAILURE_CUSTOM);
				break;
        }
    }


}
