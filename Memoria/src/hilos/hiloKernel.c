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

    t_asignarPaginasKernel* data;

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
                almacenarCodigo(pid, paginasCodigo, codigo); //esto no va a aca
                //codigo y stack o stack y heap
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
            	solicitarBytesDePagina(pid, pagina, offset, tamanio);
            	break;
            default:
				log_error(logger, "Exit por hilo Kernel");
				log_error(logger, "Tiro un exit(EXIT_FAILURE_CUSTOM) desde hilo-Kernel");
            	exit(EXIT_FAILURE_CUSTOM);
				break;
        }
    }

    /*socketClienteTemp = malloc(sizeof(int));
    *socketClienteTemp = socketCliente;
    pthread_t conexionKernel;
    pthread_create(&conexionKernel, NULL, hiloConexionKernel, (void*)socketCliente);
    */
}
