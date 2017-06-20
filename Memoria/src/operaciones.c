/*
 * operaciones.c
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */


#include "operaciones.h"

void retardo() {
	sleep(retardoUpdate);
}

/******\
|KERNEL|
\******/

void inicializarProceso(int* pid, int* paginasRequeridas) {

	//KERNEL ME PIDE INICIALIZAR UN PROCESO
	//retardo();

	log_info(logger, "Inicializando proceso: %i", pid);
	log_info(logger, "Reservando %i paginas para PID: %i...", paginasRequeridas, pid);

	if(paginasDisponibles(paginasRequeridas)) {
		printf("adentro if paginas disponibles\n");
		reservarPaginas(pid, paginasRequeridas);
		printf("despues de reservar\n");
		int * basura = 1;
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_OK, sizeof(int), &basura); //EL DATA ESTA AL PEDO PERO BUEN
		log_info(logger, "Reservadas %i paginas para PID: %i", paginasRequeridas, pid);
	} else {
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_FALLO, sizeof(int), -1); //EL DATA ESTA AL PEDO PERO BUEN
		log_error(logger, "No se pueden reservar %i paginas para PID: %i", paginasRequeridas, pid);
	}
}

void finalizarProceso(int pid) {
	//KERNEL PIDE FINALIZAR UN PROCESO
	retardo();
	log_info(logger, "Finalizando PID: %i...", pid);

	log_info(logger, "Finalizando PID: %i de cache...", pid);
	liberarProcesoDeCache(pid);

	log_info(logger, "Finalizando PID: %i de memoria principal", pid);
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid) {
			log_info(logger, "Eliminando entrada %i de la tabla de paginas del PID: %i", i, pid);
			entrada->pid = -1;
			entrada->pagina = 0;
		}
	}

	log_info(logger, "PID %i finalizado", pid);
}

void asignarPaginasAProceso(int pid, int paginasRequeridas) {
	//KERNEL PIDE ASIGNAR PAGINAS A UN PROCESO
	retardo();

	log_info(logger,"Se piden %i paginas para el proceso %i.",paginasRequeridas, pid);

	if(paginasDisponibles(paginasRequeridas)) {
		reservarPaginas(pid, paginasRequeridas);

		enviar(socketKernel, ASIGNAR_PAGINAS_OK, sizeof(int), 1); //EL DATA ESTA AL PEDO PERO BUEN

		log_info(logger,"Se reservaron %i paginas al proceso %i.", paginasRequeridas, pid);
	} else {
		enviar(socketKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), -1); //EL DATA ESTA AL PEDO PERO BUEN
		log_warning(logger, "El proceso %i no se pudo inicializar.", pid);
	}
}

/***\
|CPU|
\***/

void solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	//PEDIDO DE LECTURA POR PARTE DE CPU
	retardo();

	//leer

}

void almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
	retardo();

}

