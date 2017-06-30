/*
 * tablaDePaginas.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "tablaDePaginas.h"

void inicializarTablaDePaginas() {
	log_info(logger,"Inicializando tabla de paginas...");

	//Ubico el puntero a tabla de paginas donde inicia la memoria
	tablaDePaginas = &memoria[0];

	//Un for para ir escribiendo la tabla de pagians en memoria
	int nroDeFrameTablaDePaginas;
	for(nroDeFrameTablaDePaginas = 0; nroDeFrameTablaDePaginas <= frames; nroDeFrameTablaDePaginas++) {
		t_entradaTablaDePaginas* entradaTablaDePaginas = malloc(sizeof(t_entradaTablaDePaginas));

		int frame = nroDeFrameTablaDePaginas;
		int pid = -1;
		int pagina = 0;

		entradaTablaDePaginas->frame = frame;
		entradaTablaDePaginas->pid = pid;
		entradaTablaDePaginas->pagina = pagina;

		memcpy(&tablaDePaginas[entradaTablaDePaginas->frame], entradaTablaDePaginas, sizeof(t_entradaTablaDePaginas));

		free(entradaTablaDePaginas);
	}

	log_info(logger,"Tabla de paginas inicializada.");
}

int escribirTablaDePaginas(int frame, int pid, int pagina) {
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(frame);
	if(entrada == EXIT_FAILURE) {
		log_error(logger, "No se puede escribir en tabla de paginas. Frame: %i, PID: %i, Pagina: %i", frame, pid, pagina);
		return EXIT_FAILURE;
	} else {
		entrada->pid = pid;
		entrada->pagina = pagina;
		log_debug(logger, "Se escrbio en tabla de paginas. Frame: %i, PID: %i, Pagina: %i", frame, pid, pagina);
		return EXIT_SUCCESS;
	}
}

t_entradaTablaDePaginas* getEntradaTablaDePaginas(int index) {

	if(index > frames) {
		log_error(logger,"Se solicito una entrada inexistente");
		return EXIT_FAILURE;
	}

	return &tablaDePaginas[index];
}

t_entradaTablaDePaginas* getEntradaTablaDePaginasHash(int pid, int pagina) {
	int posiblePosicion = calcularPosicion(pid, pagina);
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(posiblePosicion);
	if(entrada->pid == pid && entrada->pagina == pagina) {
		log_info(logger, "Se encontro la entrada a la tabla de paginas para el PID: %i , Pagina: %i", pid, pagina);
		return entrada;
	}
	else {
		log_warning(logger, "Colision en funcion de hash, me voy para arriba");
		int i;
		for(i = posiblePosicion; i <= tablaDePaginasSize(); i++) {
			t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid == pid && entrada->pagina == pagina) {
				log_info(logger, "Se encontro la entrada a la tabla de paginas para el PID: %i , Pagina: %i", pid, pagina);
				return entrada;
			}
		}
	}
	int j;
	for(j = posiblePosicion; j >= 0; j--) {
		log_warning(logger, "Colision en funcion de hash, me voy para abajo");
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid == pid && entrada->pagina == pagina) {
			log_info(logger, "Se encontro la entrada a la tabla de paginas para el PID: %i , Pagina: %i", pid, pagina);
			return entrada;
		}
	}

	log_error(logger, "No se encontro la entrada a la tabla de paginas para el PID: %i, Pagina: %i", pid, pagina);
	return EXIT_FAILURE;
}

void escribirTablaDePaginasHash(int pid, int pagina) {
	int posiblePosicion = calcularPosicion(pid, pagina);
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(posiblePosicion);
	if(entrada->pid != -1) {
		entrada->pid = pid;
		entrada->pagina = pagina;
	}
	else {
		int i;
		for(i = posiblePosicion; i <= tablaDePaginasSize(); i++) {
			t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid != -1) {
				entrada->pid = pid;
				entrada->pagina = pagina;
			}
		}
	}
	int j;
	for(j = posiblePosicion; j >= 0; j--) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid != -1) {
			entrada->pid = pid;
			entrada->pagina = pagina;
		}
	}

};

int getFrameDisponibleHash(int pid, int pagina) {
	int posibleFrame = calcularPosicion(pid, pagina);
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(posibleFrame);
	if(entrada->pid == -1)
		return posibleFrame;
	else {
		log_warning(logger, "Colision en funcion de hash, me voy para arriba");
		int i;
		for(i = posibleFrame + 1; i <= tablaDePaginasSize(); i++) {
			entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid == -1) {
				return i;
			}
		}
	}
	int j;
	log_warning(logger, "No hay espacios para arriba, me voy para abajo");
	for(j = posibleFrame - 1; j >= 0; j--) {
		entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid == -1) {
			return j;
		}
	}
	log_error(logger, "No hay frames disponibles");
	return EXIT_FAILURE;
}

bool paginasDisponibles(int paginasRequeridas) {
	if(getCantidadFramesDisponibles() >= paginasRequeridas) {
		return true;
	} else {
		log_error(logger, "No hay mas espacio disponible en memoria");
		return false;
	}
	/*
	int i;
	for(i = 1; i <= paginasRequeridas; i++) {
		if(getFrameDisponible() == -1) {
			log_warning(logger, "No hay mas espacio disponible en memoria.");
			return false;
		}

	log_debug(logger, "Hay espacio disponible");
	return true;
	}*/
}

int getFrameDisponible() {
	int i;
	int frameDisponible = -1;
	for(i = 0; i <= frames; i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == -1)
			return entrada->frame;
	}

	return frameDisponible;
}

int getCantidadFramesDisponibles() {
	int i;
	int framesDisponibles = 0;
	for(i = 0; i < frames; i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == -1)
			framesDisponibles++;
	}

	return framesDisponibles;
}

int getCantidadFramesOcupados() {
	return frames - getCantidadFramesDisponibles();
}

int tablaDePaginasSize() {
	return frames;
}

void reservarPaginas(int pid, int paginasAReservar) {
	int i;
	for(i = 1; i <= paginasAReservar; i++) {
		int pagina = i;
		int frameDisponible = getFrameDisponibleHash(pid, pagina);
		escribirTablaDePaginas(frameDisponible, pid, pagina);
	}
}

int asignarMasPaginasAProceso(int pid, int paginasAsignar) {
	int ultimaPagina = getUltimaPagina(pid);
	if(ultimaPagina == EXIT_FAILURE) {
		log_error(logger, "No se pueden asignar %i paginas al PID %i",paginasAsignar, pid);
		return EXIT_FAILURE;
	}
	int i;
	for(i = 0; i < paginasAsignar; i++) {
		ultimaPagina++;
		int pagina = ultimaPagina;
		int frameDisponible = getFrameDisponibleHash(pid, pagina);
		if(frameDisponible != EXIT_FAILURE) {
			int exito = escribirTablaDePaginas(frameDisponible, pid, pagina);
			if(exito == EXIT_SUCCESS) {
				log_info(logger, "Se asigno una pagina mas para el PID %i. PID: %i, ultima pagina asignada: %i", pid, pid, pagina);
			} else {
				log_error(logger, "No se  pudo asignar una pagina mas para el PID %i. PID: %i, ultima pagina que quizo ser asignada: %i", pid, pid, pagina);
				return EXIT_FAILURE;
			}
		} else {
			log_error(logger, "No hay mas espacio para asignar la pagina %i del PID %i", pid, pagina);
			log_error(logger, "No se puedieron asignar todas las pagnas requeridas");
			return EXIT_FAILURE;
		}
	}

	log_debug(logger, "Se asignaron %i paginas mas para el PID %i", paginasAsignar, pid);
	return EXIT_SUCCESS;
}

int getUltimaPagina(int pid) {
	int ultimaPagina = -1;
	t_list* entradasPID = getEntradasDePID(pid);
	int i;
	for(i = 0; i < entradasPID->elements_count; i++) {
		t_entradaTablaDePaginas* entrada = list_get(entradasPID, i);
		if(entrada->pagina > ultimaPagina)
			ultimaPagina = entrada->pagina;
	}

	if(ultimaPagina == -1) {
		log_error(logger, "No pudo encontrar la ultima pagina del PID %i", pid);
		return EXIT_FAILURE;
	}

	log_debug(logger, "Ultima pagina PID %i: %i", pid, ultimaPagina);
	return ultimaPagina;
}

t_list* getEntradasDePID(int pid) {
	int i;
	t_list* lista = list_create();
	for(i = 0; i < frames; i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid)
			list_add(lista, entrada);
	}

	if(lista->elements_count <= 0) {
		log_error(logger, "No se encontraron entradas del PID %i", pid);
		return EXIT_FAILURE;
	}


	return lista;
}

int esPaginaLiberable(int pid, int pagina) {
	return EXIT_SUCCESS;
}

int liberarPagina(int pid, int pagina) {
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginasHash(pid, pagina);
	if(entrada == EXIT_FAILURE) {
		log_error(logger, "No se puede liberar pagina nro %i del PID %i", pagina, pid);
		return EXIT_FAILURE;
	}

	entrada->pid = -1;
	entrada->pagina = 0;

	log_debug(logger, "Se libero la pagina nro %i del PID %i", pagina, pid);
	return EXIT_SUCCESS;
}

bool existePIDPagina(int pid, int pagina) {
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) { //VER SI ES <= O < Y SI INICIALIZA EN 0
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid && entrada->pagina == pagina)
			return true;
	}

	log_warning(logger, "No existe PID %i Pagina %i en tabla de paginas", pid, pagina);
	return false;
}

int getFrameByPIDPagina(int pid, int pagina) {
	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) { //VER SI ES <= O < Y SI INICIALIZA EN 0
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid == pid && entrada->pagina == pagina)
			return entrada->frame;
	}

	log_warning(logger, "No existe PID %i Pagina %i en tabla de paginas", pid, pagina);
	return EXIT_FAILURE_CUSTOM;
}
