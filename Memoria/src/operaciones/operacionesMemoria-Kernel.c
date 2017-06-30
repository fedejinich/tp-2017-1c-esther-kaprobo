/*
 * operaciones.c
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */


#include "operacionesMemoria-Kernel.h"

/******\
|KERNEL|
\******/

void inicializarProceso(int pid, int paginasRequeridas) {
	retardo();

	log_info(logger, "Inicializando proceso: %i", pid);
	log_info(logger, "Reservando %i paginas para PID: %i...", paginasRequeridas, pid);

	if(paginasDisponibles(paginasRequeridas)) {
		reservarPaginas(pid, paginasRequeridas);
		int * ok = 1; //si saco esto y dejo el 1  en el paquete me tira segmentation fault
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_OK, sizeof(int), &ok); //EL DATA ESTA AL PEDO PERO BUEN
		log_debug(logger, "Reservadas %i paginas para PID: %i", paginasRequeridas, pid);
	} else {
		int * fallo = -1; //si saco esto y dejo el -1  en el paquete me tira segmentation fault
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_FALLO, sizeof(int), &fallo); //EL DATA ESTA AL PEDO PERO BUEN
		log_error(logger, "No se pueden reservar %i paginas para PID: %i", paginasRequeridas, pid);
	}

}

void finalizarProceso(int pid) {
	retardo();
	log_warning(logger, "Finalizando PID: %i...", pid);

	log_info(logger, "Finalizando PID: %i de cache...", pid);
	liberarProcesoDeCache(pid);

	log_info(logger, "Finalizando PID: %i de memoria principal", pid);
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid) {
			log_warning(logger, "Eliminando entrada %i de la tabla de paginas del PID: %i", i, pid);
			entrada->pid = -1;
			entrada->pagina = 0;
		}
	}

	log_debug(logger, "PID %i finalizado", pid);
}

void asignarPaginasAProceso(int pid, int paginasAsignar) {
	retardo();

	log_info(logger,"Se piden %i paginas mas para el proceso %i.",paginasAsignar, pid);

	if(paginasDisponibles(paginasAsignar)) {
		int exito = asignarMasPaginasAProceso(pid, paginasAsignar);
		if(exito == EXIT_SUCCESS) {
			int* ok = 1;
			enviar(socketKernel, ASIGNAR_PAGINAS_OK, sizeof(int), ok); //EL DATA ESTA AL PEDO PERO BUEN
			log_debug(logger,"Se asignaron %i paginas mas al PID: %i", pid, paginasAsignar);
		} else {
			int* fallo = -1;
			enviar(socketKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), fallo);
			log_error(logger, "No se pudieron asignar %i paginas a PID %i", paginasAsignar, pid);
		}
	} else {
		enviar(socketKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), -1); //EL DATA ESTA AL PEDO PERO BUEN
		log_error(logger, "No hay espacio disponible para asignar %i paginas mas al PID %i", paginasAsignar, pid);
	}
}

void liberarPaginaProceso(int pid, int pagina) {
	/****************************************************************************************************
	 * LIBERAR PAGINA DE UN PROCESO																		*
	 * Parámetros: [Identificador del Programa] [Número de Página elegida]								*
	 * Ante un pedido de liberación de página por parte del kernel, el proceso memoria deberá liberar	*
	 * la página que corresponde con el número solicitado. En caso de que dicha página no exista		*
	 * o no pueda ser liberada, se deberá informar de la imposibilidad de realizar dicha operación		*
	 * como una excepcion de memoria.																	*
	 ***************************************************************************************************/

	//MI PREGUNTA ES: POR QUE NO SE PODRIA LIBERAR UNA PAGINA?
	retardo();

	log_warning(logger, "Liberando la pagina nro %i del PID %i...", pagina, pid);
	if(esPaginaLiberable(pid, pagina)) {
		int exito = liberarPagina(pid, pagina);
		if(exito == EXIT_SUCCESS) {
			int* ok = 1;
			enviar(socketKernel, LIBERAR_PAGINA_OK, sizeof(int), ok);
			log_debug(logger, "Se libero la pagina nro %i del PID %i", pagina, pid);
		} else {
			int* fallo = -1;
			enviar(socketKernel, LIBERAR_PAGINA_FALLO, sizeof(int), fallo);
			log_error(socketKernel, "No se pudo liberar la pagina nro %i del PID %i", pagina, pid);
		}
	}

}


