/*
 * frames.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "frames.h"

void escribirFrame(int frame, int offset, int tamanio, void * contenido) {

	//Es el desplazamiento dentro del bloque de memoria principal para luego conseguir el frame en el cual voy a escribir
	//int desplazamiento = frame * frame_size;
	sleep(retardo_memoria); //retardo de memoria

	memcpy(&framePointer[frame] + offset, contenido, tamanio);

}

int cantidadDeFramesOcupados() {
	int i;
	int cantidad = 0;
	for(i = 0; i <= frames; i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		if(entrada->pid != -1)
			cantidad++;
	}

	return cantidad;
}

void liberarFrame(int frame) {
	t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(frame);
	entrada->pid = -1;
	entrada->pagina = 0;
}

void inicializarFramePointer() {
	framePointer = &memoria[getFirstFrame()];
}

int getFirstFrame() {
	int tablaDePaginasSize = frames * sizeof(t_entradaTablaDePaginas);
	int index;
	double t = (double)(((double)tablaDePaginasSize) / ((double)frame_size)); // la expresividad: me la meti en el orto
	double i = getParteDecimal(t);
	if(i > 0)
		index = ((int) (getParteEntera(t))) + 1;
	else
		index = ((int) (getParteEntera(t)));

	return index;
}


