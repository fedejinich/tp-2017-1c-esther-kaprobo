/*
 * operaciones.c
 *
 *  Created on: 5/7/2017
 *      Author: utnso
 */


#include "operaciones.h"

void retardo() {
	log_info(logger, "sleep(%i)", retardo_memoria);
	usleep(retardo_memoria * 1000);
}

void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	log_info(logger, "Solicitando bytes de PID: %i, pagina: %i, offset: %i y tamanio: %i", pid, pagina, offset, tamanio);

	if(!superaLimiteFrame(offset, tamanio)) {
		void* buffer;

		if(cache_x_proc > 0 && estaEnCache(pid, pagina)) {
			buffer = leerDeCache(pid, pagina, offset, tamanio);
		} else {
			log_info(logger, "Solicitando bytes en memoria..");
			retardo();

			t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginasHash(pid, pagina);

			if(entrada == EXIT_FAILURE_CUSTOM) {
				log_error(logger, "Error en solicitar bytes de pagina %i PID %i", pagina, pid);
				return EXIT_FAILURE_CUSTOM;
			}

			log_info(logger, "Los bytes del PID: %i, pagina: %i se encuentran en el frame %i", pid, pagina, entrada->frame);


			buffer = malloc(tamanio);
			leerFrame(entrada->frame, offset, tamanio, buffer);

			if(buffer == EXIT_FAILURE_CUSTOM) {
				free(buffer);
				log_error(logger, "No se pudo cumplir la solicutd de bytes. PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);
				return EXIT_FAILURE_CUSTOM;
			}

			void* bufferParaCache = getPaginaByPID(pid, pagina);

			if(cache_x_proc > 0) {
				escribirCache(pid, pagina, tamanio, bufferParaCache);
			}
		}

		return buffer;
	}

	log_error(logger, "Error al solicitar bytes PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);
	return EXIT_FAILURE_CUSTOM;
}

int almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	if(pid == 1 && pagina == 411) {
		dumpTabla();
		dumpCache();
	}

	retardo();

	//tamanio--; //por el \0

	log_info(logger, "Almacenando %i bytes de PID %i en pagina %i con offset %i ...", tamanio, pid, pagina, offset);

	int frame = getFrameByPIDPagina(pid, pagina);

	if(frame == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se pudieron almacenar %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);
		return EXIT_FAILURE_CUSTOM;
	}

	log_info(logger, "Frame en el que se va a almacenar %i", frame);

	if(!superaLimiteFrame(offset, tamanio)) {
		escribirFrame(frame, offset, tamanio, buffer);
		log_debug(logger, "Almacenados %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);

		if(estaEnCache(pid, pagina)) {
			void* bufferAReplicarEnCache = getPaginaByPID(pid, pagina);
			escribirCache(pid, pagina, frame_size, bufferAReplicarEnCache);
		}

		return EXIT_SUCCESS_CUSTOM;
	}

	log_error(logger, "El offset %i con tamanio %i supera el limite de escritura en frame %i", offset, tamanio, frame);
	log_error(logger, "No se pudieron almacenar %i bytes en frame %i de PID %i en pagina %i con offset %i", tamanio, frame,pid, pagina, offset);
	return EXIT_FAILURE_CUSTOM;
}

int inicializarProceso(int pid, int paginasRequeridas) {
	retardo();

	log_warning(logger, "Inicializando proceso. PID %i ...", pid);
	if(paginasRequeridas > 1)
		log_info(logger, "Reservando %i paginas ...", paginasRequeridas);
	else
		log_info(logger, "Reservando una pagina");

	if(paginasDisponibles(pid, paginasRequeridas)) {
		int exito = reservarPaginas(pid, paginasRequeridas);

		if(exito == EXIT_FAILURE_CUSTOM) {
			int* fallo = (int*) EXIT_FAILURE_CUSTOM; //si saco esto y dejo el -1  en el paquete me tira segmentation fault
			enviar(socketClienteKernel, INICIALIZAR_PROCESO_FALLO, sizeof(int), &fallo); //EL DATA ESTA AL PEDO PERO BUEN
			if(paginasRequeridas > 1)
				log_error(logger, "No se pueden reservar %i paginas para PID %i", paginasRequeridas, pid);
			else
				log_error(logger, "No se puede reservar una pagina para PID %i", pid);

			return EXIT_FAILURE_CUSTOM;
		}

		int* ok = (int*) EXIT_SUCCESS_CUSTOM;
		enviar(socketClienteKernel, INICIALIZAR_PROCESO_OK, sizeof(int), &ok); //EL DATA ESTA AL PEDO PERO BUEN

		t_ultimaPaginaPID* entradaUltimaPagina = malloc(sizeof(t_ultimaPaginaPID));
		entradaUltimaPagina->pid = pid;
		entradaUltimaPagina->ultimaPagina = paginasRequeridas;
		list_add(ultimasPaginasDePIDs, entradaUltimaPagina);

		if(paginasRequeridas > 1)
			log_info(logger, "Reservadas %i paginas para PID %i", paginasRequeridas, pid);
		else
			log_info(logger, "Reservada una pagina para PID %i", pid);

		log_debug(logger, "Proceso inicializado correctamente. PID %i", pid);

		return EXIT_SUCCESS_CUSTOM;
	}

	int* fallo = (int*) -1; //si saco esto y dejo el -1  en el paquete me tira segmentation fault
	enviar(socketClienteKernel, INICIALIZAR_PROCESO_FALLO, sizeof(int), &fallo); //EL DATA ESTA AL PEDO PERO BUEN
	if(paginasRequeridas > 1)
		log_error(logger, "No se pueden reservar %i paginas para PID %i", paginasRequeridas, pid);
	else
		log_error(logger, "No se puede reservar una pagina para PID %i", pid);

	return EXIT_FAILURE_CUSTOM;
}

int finalizarProceso(int pid) {
	retardo();

	//dumpPID(string_itoa(pid));

	log_warning(logger, "Finalizando proceso. PID %i ...", pid);

	log_info(logger, "Finalizando proceso de cache. PID %i ...", pid);
	liberarProcesoDeCache(pid);

	log_info(logger, "Finalizando proceso de memoria principal. PID %i ...", pid);

	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid) {
			escribirTablaDePaginas(entrada->frame, -1,-1);
			log_info(logger, "Liberada de tabla de paginas, PID %i Pagina %i", pid, entrada->pagina);
		}
	}
/*
	t_list* entradasDePID = getEntradasDePID(pid);

	if(entradasDePID == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se pudo finalizar proceso. PID %i", pid);
		return EXIT_FAILURE_CUSTOM;
	}

	int i;
	for(i = 0; i <= entradasDePID->elements_count; i++) {
		t_entradaTablaDePaginas* entrada = list_get(entradasDePID, i);

		log_info(logger, "Eliminando de tabla de paginas PID %i, Pagina %i", entrada->pid, entrada->pagina);

		escribirTablaDePaginas(entrada->frame, -1, -1);

	}*/

	log_debug(logger, "Proceso finalizado. PID %i", pid);
	return EXIT_SUCCESS_CUSTOM;
}


int asignarPaginasAProceso(int pid, int paginasAsignar) {
	retardo();

	if(paginasAsignar > 1)
		log_warning(logger,"Asignando %i paginas al proceso. PID %i ...",paginasAsignar, pid);
	else
		log_warning(logger, "Asignando una pagina al proceso. PID %i", pid);

	int paginasDisponiblesOk = paginasDisponibles(pid, paginasAsignar);

	if(paginasDisponiblesOk == EXIT_FAILURE_CUSTOM) {
		if(paginasAsignar > 1)
			log_error(logger, "No se pudieron asignar %i paginas al proceso. PID %i", paginasAsignar, pid);
		else
			log_error(logger, "No se pudo asignar una pagina al proceso. PID %i", pid);

		dumpTabla();
		dumpCache();

		return EXIT_FAILURE_CUSTOM;
	}

	int exito = asignarMasPaginasAProceso(pid, paginasAsignar);

	if(exito == EXIT_FAILURE_CUSTOM) {
		if(paginasAsignar > 1)
			log_error(logger, "No se pudieron asignar %i paginas al proceso. PID %i", paginasAsignar, pid);
		else
			log_error(logger, "No se pudo asignar una pagina al proceso. PID %i", pid);

		dumpTabla();
		dumpCache();

		return EXIT_FAILURE_CUSTOM;
	}

	return EXIT_SUCCESS_CUSTOM;
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
			int fallo ;
			enviar(socketClienteKernel, LIBERAR_PAGINA_FALLO, sizeof(int), fallo);
			log_error(logger, "No se pudo liberar la pagina nro %i del proceso. PID %i", pagina, pid);

			return EXIT_FAILURE_CUSTOM;
		}

		int ok ;
		enviar(socketClienteKernel, LIBERAR_PAGINA_OK, sizeof(int), ok);
		log_debug(logger, "Se libero la pagina nro %i del proceso. PID %i", pagina, pid);

		return EXIT_SUCCESS_CUSTOM;
	}

	log_error(logger, "No se libero la pagina nro %i del proceso. PID %i", pagina, pid);

	return EXIT_FAILURE_CUSTOM;
}
