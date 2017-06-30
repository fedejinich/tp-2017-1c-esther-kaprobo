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

int inicializarProceso(int pid, int paginasRequeridas) {
	retardo();

	log_warning(logger, "Inicializando proceso. PID %i ...", pid);
	if(paginasRequeridas > 1)
		log_info(logger, "Reservando %i paginas ...", paginasRequeridas);
	else
		log_info(logger, "Reservando una pagina");

	if(paginasDisponibles(pid, paginasRequeridas)) {
		reservarPaginas(pid, paginasRequeridas);
		int* ok = (int*) 1; //si saco esto y dejo el 1  en el paquete me tira segmentation fault
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_OK, sizeof(int), &ok); //EL DATA ESTA AL PEDO PERO BUEN
		if(paginasRequeridas > 1)
			log_info(logger, "Reservadas %i paginas para PID %i", paginasRequeridas, pid);
		else
			log_info(logger, "Reservada una pagina para PID %i", pid);
		log_debug(logger, "Proceso inicializado correctamente. PID %i", pid);

		return EXIT_SUCCESS_CUSTOM;
	} else {
		int* fallo = (int*) -1; //si saco esto y dejo el -1  en el paquete me tira segmentation fault
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_FALLO, sizeof(int), &fallo); //EL DATA ESTA AL PEDO PERO BUEN
		if(paginasRequeridas > 1)
			log_error(logger, "No se pueden reservar %i paginas para PID %i", paginasRequeridas, pid);
		else
			log_error(logger, "No se puede reservar una pagina para PID %i", pid);

		return EXIT_FAILURE_CUSTOM;
	}

}

int finalizarProceso(int pid) {
	retardo();
	log_warning(logger, "Finalizando proceso. PID %i ...", pid);

	log_info(logger, "Finalizando proceso de cache. PID %i ...", pid);
	liberarProcesoDeCache(pid);

	log_info(logger, "Finalizando proceso de memoria principal. PID %i ...", pid);
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "No se pudo finalizar proceso. PID %i", pid);
			return EXIT_FAILURE_CUSTOM;
		}

		if(entrada->pid == pid) {
			log_info(logger, "Eliminando entrada %i de la tabla de paginas del PID: %i...", i, pid);
			entrada->pid = -1;
			entrada->pagina = 0;
		}
	}

	log_debug(logger, "Proceso finalizado. PID %i", pid);
	return EXIT_SUCCESS_CUSTOM;
}

int asignarPaginasAProceso(int pid, int paginasAsignar) {
	retardo();

	if(paginasAsignar > 1)
		log_warning(logger,"Asignando %i paginas al proceso. PID %i ...",paginasAsignar, pid);
	else
		log_warning(logger, "Asignando una pagina al proceso. PID %i", pid);

	bool paginasDisponiblesOk = paginasDisponibles(pid, paginasAsignar);

	if(paginasDisponiblesOk == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error al asignar paginas al proceso PID %i, Paginas a asignar %i", pid, paginasAsignar);
		return EXIT_FAILURE_CUSTOM;
	}

	if(paginasDisponiblesOk) {
		int exito = asignarMasPaginasAProceso(pid, paginasAsignar);
		if(exito == EXIT_FAILURE_CUSTOM) {
			int* fallo = (int*) -1;
			enviar(socketKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), fallo);
			if(paginasAsignar > 1)
				log_error(logger, "No se pudieron asignar %i paginas al proceso. PID %i", paginasAsignar, pid);
			else
				log_error(logger, "No se pudo asignar una pagina al proceso. PID %i", pid);

			return EXIT_FAILURE_CUSTOM;
		}

		int* ok = (int*) 1;
		enviar(socketKernel, ASIGNAR_PAGINAS_OK, sizeof(int), ok); //EL DATA ESTA AL PEDO PERO BUEN
		if(paginasAsignar > 1)
			log_debug(logger, "Se asignaron %i paginas mas al proceso. PID: %i", paginasAsignar, pid);
		else
			log_debug(logger, "Se asigno una pagina mas al proceso. PID %i", pid);

		return EXIT_SUCCESS_CUSTOM;

	} else {
		int* fallo = (int*) -1;
		enviar(socketKernel, ASIGNAR_PAGINAS_FALLO, sizeof(int), fallo); //EL DATA ESTA AL PEDO PERO BUEN
		if(paginasAsignar > 1)
			log_error(logger, "No se pudieron asignar %i paginas al proceso. PID %i", paginasAsignar, pid);
		else
			log_error(logger, "No se pudo asignar una pagina al proceso. PID %i", pid);

		return EXIT_FAILURE_CUSTOM;
	}
}

int liberarPaginaProceso(int pid, int pagina) {
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

	log_warning(logger, "Liberando pagina nro %i del proceso. PID %i ...", pagina, pid);

	if(esPaginaLiberable(pid, pagina)) {
		int exito = liberarPagina(pid, pagina);
		if(exito == EXIT_FAILURE_CUSTOM) {
			int* fallo = (int*) -1;
			enviar(socketKernel, LIBERAR_PAGINA_FALLO, sizeof(int), fallo);
			log_error(logger, "No se pudo liberar la pagina nro %i del proceso. PID %i", pagina, pid);

			return EXIT_FAILURE_CUSTOM;
		}

		int* ok = (int*) 1;
		enviar(socketKernel, LIBERAR_PAGINA_OK, sizeof(int), ok);
		log_debug(logger, "Se libero la pagina nro %i del proceso. PID %i", pagina, pid);

		return EXIT_SUCCESS_CUSTOM;
	}

	log_error(logger, "No se libero la pagina nro %i del proceso. PID %i", pagina, pid);

	return EXIT_FAILURE_CUSTOM;
}


