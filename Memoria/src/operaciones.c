/*
 * operaciones.c
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */


#include "operaciones.h"


/******\
|KERNEL|
\******/

void inicializarProceso(int pid, int paginasRequeridas) {
	//KERNEL ME PIDE INICIALIZAR UN PROCESO
}

void finalizarProceso(int pid) {
	//KERNEL PIDE FINALIZAR UN PROCESO
}

void asignarPaginasAProceso(int pid, int paginasRequeridas) {
	//KERNEL PIDE ASIGNAR PAGINAS A UN PROCESO

	log_info(logger,"Se piden %i paginas para el proceso %i.",paginasRequeridas, pid);

	if(paginasDisponibles(paginasRequeridas)) {
		int i;
		for(i = 1; i <= paginasRequeridas; i++) {
			int frameDisponible = getFrameDisponible();

			escribirTablaDePaginas(frameDisponible, pid, i);
		}

		enviar(socketKernel, ASIGNAR_PAGINAS_OK, sizeof(int), paginasRequeridas);

		log_info(logger,"Se otorgaron %i paginas al proceso %i.", paginasRequeridas, pid);
	} else {
		enviar(socketKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), -1); //EL TAMANIO Y DATA ESTAN AL PEDO PERO BUEN
		log_warning(logger, "El proceso %i no se pudo inicializar.", pid);
	}
}




/***\
|CPU|
\***/

void solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	//PEDIDO DE LECTURA POR PARTE DE CPU
}

void almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
}

