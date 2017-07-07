/*
 * frames.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "frames.h"

void escribirFrame(int frame, int offset, int tamanio, void * contenido) {
	log_info(logger, "Escribiendo frame %i con offset %i y tamanio %i", frame, offset, tamanio);

	memcpy(&framePointer[frame] + offset, &contenido, tamanio);
}

void leerFrame(int frame, int offset, int tamanio, void* buffer) {
	log_info(logger, "Leyendo frame %i con offset %i y tamanio %i", frame, offset, tamanio);

	log_warning(logger, "AAAAAAAA %s", framePointer[11]);

	void* bufferAux = malloc(tamanio);
	memcpy(&bufferAux, &framePointer[frame] + offset, tamanio);
	memcpy(&buffer, bufferAux, tamanio);
	//return buffer;
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



bool superaLimiteFrame(int offset, int tamanio) {
	bool supera = (tamanio >= frame_size) || (offset > frame_size) || ((offset + tamanio) >= frame_size);

	if(supera == true) {
		log_error(logger, "Supera limite de frame. Offset %i, Tamanio %i", offset, tamanio);
		return false;
	}

	return supera;
}


