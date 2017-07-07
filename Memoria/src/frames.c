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
	//sleep(retardo_memoria); //retardo de memoria

	printf("por memcpy\n");
	printf("ASDDDDD = %s\n", (char*)contenido);

/*	char* cot = malloc(3);
	memcpy(cot, &contenido, tamanio);
	printf("cont %s", cot);*/

	memcpy(&framePointer[frame] + offset, &contenido, tamanio);

	printf("por leer\n");
	void* lectura = leerFrame(1,0,3);
	printf("lei\n");
	printf("contenido %s\n", lectura);

	if(string_equals_ignore_case(lectura,"asd"))
		printf("bien\n");

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

void* leerFrame(int frame, int offset, int tamanio) {
	//aca funcion de hash
	void* contenido = malloc(tamanio);

	memcpy(&contenido, &framePointer[frame] + offset, tamanio);

	return contenido;
}

bool superaLimiteFrame(int offset, int tamanio) {
	log_warning(logger, "(tamanio %i <= frameSize %i) && (offset %i < frameSize %i) && ((offset %i + tamanio %i) <= frameSize %i)",
			tamanio, frame_size, offset, frame_size, offset, tamanio, frame_size);

	bool exito = (tamanio <= frame_size) && (offset < frame_size) && ((offset + tamanio) <= frame_size);

	if(exito == true)
		log_warning(logger, "Exito =  true");
	else
		log_warning(logger, "Exito =  false");

	if(exito == false) {
		log_error(logger, "Supera limite de frame. Offset %i, Tamanio %i", offset, tamanio);
		return false;
	}

	log_debug(logger, "Saliendo de superaLLimite");
	return !exito;
}


