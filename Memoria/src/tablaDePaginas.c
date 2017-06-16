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

		escribirTablaDePaginas(entradaTablaDePaginas);

		free(entradaTablaDePaginas);
	}

	log_info(logger,"Tabla de paginas inicializada.");
}

void escribirTablaDePaginas(t_entradaTablaDePaginas*  entrada) {
	memcpy(&tablaDePaginas[entrada->frame], entrada, sizeof(t_entradaTablaDePaginas));
}

t_entradaTablaDePaginas* getEntradaTablaDePaginas(int index) {

	if(index > frames) {
		log_error(logger,"Se solicito una entrada inexistente");
		return EXIT_FAILURE;
	}

	return &tablaDePaginas[index];
}

bool paginasDisponibles(int paginasRequeridas) {
	int i;
	for(i = 1; i <= paginasRequeridas; i++) {
		if(getFrameDisponible() == -1) {
			log_warning(logger, "No hay mas espacio disponible en memoria.");
			return false;
		}

	}

	log_info(logger, "Hay espacio disponible");
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

int getTablaDePaginasSize() {
	return frames;
}


