/*
 * operaciones.c
 *
 *  Created on: 5/7/2017
 *      Author: utnso
 */


#include "operaciones.h"

void retardo() {
	log_info(logger, "sleep(%i)", retardo_memoria);
}

void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	retardo(); //EL RETARDO VA ACA O VA CUANDO EMPIEZA EL ELSE? LA CACHE SE AFECTA POR EL RETARDO?
	void* buffer = -911;
	log_info(logger, "Solicitando bytes de PID: %i, pagina: %i, offset: %i y tamanio: %i", pid, pagina, offset, tamanio);

	if(estaEnCache(pid, pagina))
		leerDeCache(pid, pagina);
	else {
		retardo();
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginasHash(pid, pagina);

		if(entrada == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "Error en solicitar bytes de una pagina");
			return EXIT_FAILURE_CUSTOM;
		}

		log_info(logger, "Los bytes del PID: %i, pagina: %i se encuentran en el frame %i", pid, pagina, entrada->frame);
		void* buffer = leerFrame(entrada->frame, offset, tamanio);

		if(buffer == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "No se pudo cumplir la solicutd de bytes. PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);
			return EXIT_FAILURE_CUSTOM;
		}

		escribirCache(pid, pagina, buffer);
	}

	if(buffer == -911) {
		log_error(logger, "No se pudo cumplir la solicutd de bytes. PID %i, Pagina %i, Offset %i, Tamanio %i", pid, pagina, offset, tamanio);
		return EXIT_FAILURE_CUSTOM;
	}

	return buffer;
}

int almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//log_warning(logger, "Buffer %s", (char*) buffer);
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
	retardo();
	log_info(logger, "Almacenando %i bytes de PID %i en pagina %i con offset %i ...", tamanio, pid, pagina, offset);

	int frame = getFrameByPIDPagina(pid, pagina);

	if(frame == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se pudieron almacenar %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);
		return EXIT_FAILURE_CUSTOM;
	}

	log_warning(logger, "Frame en el que se van a almacenar %i", frame);

	bool noSuperaLimite = !superaLimiteFrame(offset, tamanio);

	if(noSuperaLimite) {
		log_debug(logger, "no supera limite");
	} else {
		log_error(logger, "supera limite");
	}

	if(noSuperaLimite) {
		//log_warning(logger, "escribirFrame(%i, %i, %i, %s)", frame, offset, tamanio, (char*)buffer);
		escribirFrame(frame, offset, tamanio, buffer);
		log_debug(logger, "Almacenados %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);

		return EXIT_SUCCESS_CUSTOM;
	}

	log_error(logger, "No se pudieron almacenar %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);
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

int almacenarCodigo(int pid, int paginasCodigo, char* codigo) {
	int paginasCodigoVerificador = cantidadPaginasCodigo(codigo);

	if(paginasCodigoVerificador != paginasCodigo) {
		log_error(logger, "Error en almacenarCodigo(%i, %i, %s)", pid, paginasCodigo, codigo);
		log_error(logger, "La cantidad de paginas que desea almacenar es incorrecta");
		return EXIT_FAILURE_CUSTOM;
	}

	int primeraPagina = getFramePrimeraPagina(pid);

	t_list* codigosParciales = getCodigosParciales(codigo, frame_size);

	int i;
	for(i = 0; i < paginasCodigo; i++) {
		char* codigoParcial = list_get(codigosParciales, i);
		int tamanioCodigoParcial = strlen(codigoParcial);
		log_warning(logger, "escribirFrame(%i + %i, 0, %i, %i", primeraPagina, i, tamanioCodigoParcial, strlen(codigoParcial));
		escribirFrame(primeraPagina + i, 0, tamanioCodigoParcial, codigoParcial);
	}

	log_info(logger, "Codigo almacenado en memoria");
	return EXIT_SUCCESS_CUSTOM;
}
