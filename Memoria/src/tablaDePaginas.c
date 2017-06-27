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

void escribirTablaDePaginas(int frame, int pid, int pagina) {
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(frame);
	entrada->pid = pid;
	entrada->pagina = pagina;
}

t_entradaTablaDePaginas* getEntradaTablaDePaginas(int index) {

	if(index > frames) {
		log_error(logger,"Se solicito una entrada inexistente");
		return EXIT_FAILURE;
	}

	return &tablaDePaginas[index];
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
	if(entrada->pid == -1) {
		log_debug(logger, "Frame disponible. Frame: %i", posibleFrame);
		return posibleFrame;
	} else {
		log_warning(logger, "Colision en funcion de hash, me voy para arriba");
		int i;
		for(i = posibleFrame + 1; i <= tablaDePaginasSize(); i++) {
			entrada = getEntradaTablaDePaginas(i);
			if(entrada->pid == -1) {
				log_debug(logger, "Frame disponible. Frame: %i", i);
				return i;
			}
		}
	}
	int j;
	log_warning(logger, "No hay espacios para arriba, me voy para abajo");
	for(j = posibleFrame - 1; j >= 0; j--) {
		entrada = getEntradaTablaDePaginas(j);
		if(entrada->pid == -1) {
			log_debug(logger, "Frame disponible. Frame: %i", j);
			return j;
		}
	}
	log_error(logger, "No hay frames disponibles");
	return EXIT_FAILURE;
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

bool paginasDisponibles(int paginasRequeridas) {
	int i;
	for(i = 1; i <= paginasRequeridas; i++) {
		if(getFrameDisponible() == -1) {
			log_warning(logger, "No hay mas espacio disponible en memoria.");
			return false;
		}

	}

	log_debug(logger, "Hay espacio disponible");
	return true;
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
		int frameDisponible = getFrameDisponibleHash(pid, pagina);//getFrameDisponible();  //aca va funcion de hash
		escribirTablaDePaginas(frameDisponible, pid, i);
	}
}


