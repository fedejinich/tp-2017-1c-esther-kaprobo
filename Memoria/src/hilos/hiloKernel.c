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
    log_info(logger,"Creacion socket servidor Kernel exitosa");
    listen(socketKernel, 1024);


    socketClienteKernel = aceptar_conexion(socketKernel);
    log_info(logger,"Iniciando Handshake con Kernel\n");
    bool resultado_hand = esperar_handshake(socketClienteKernel,13);
    if(resultado_hand){
        log_info(logger,"Conexión aceptada del Kernel %d!!", socketClienteKernel);
        enviar(socketClienteKernel,TAMANIO_PAGINA,sizeof(int),&frame_size);
    } else {
        log_info(logger,"Handshake fallo, se aborta conexion");
        exit (EXIT_FAILURE);
    }

    t_paquete * paqueteRecibido;
    int* pid, paginasRequeridas;
    int  pagina, tamanio, offset, tamanioCodigo;
    void* buffer;

    while (1) {
        paqueteRecibido = recibir(socketClienteKernel);
        char * codigoDeOperacion = getCodigoDeOperacion(paqueteRecibido->codigo_operacion);

        log_info(logger, "Codigo de operacion: %s", codigoDeOperacion);
        switch (paqueteRecibido->codigo_operacion) {
            case INICIALIZAR_PROCESO:
                paginasRequeridas = ((t_pedidoDePaginas*)(paqueteRecibido->data))->paginasAPedir;
                pid = ((t_pedidoDePaginas*)(paqueteRecibido->data))->pid;
                inicializarProceso(pid, paginasRequeridas);
                break;
            case ASIGNAR_PAGINAS:
                paginasRequeridas = ((t_pedidoDePaginas*)(paqueteRecibido->data))->paginasAPedir;
                pid = ((t_pedidoDePaginas*)(paqueteRecibido->data))->pid;
                asignarPaginasAProceso(pid, paginasRequeridas);
                break;
            case FINALIZAR_PROCESO:
                finalizarProceso(pid);
                break;
            default:
				break;
        }


    }



    /*socketClienteTemp = malloc(sizeof(int));
    *socketClienteTemp = socketCliente;
    pthread_t conexionKernel;
    pthread_create(&conexionKernel, NULL, hiloConexionKernel, (void*)socketCliente);
    */
}
