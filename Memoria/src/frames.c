/*
 * frames.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "frames.h"

void escribir_frame(int frame, int offset, int tamanio, void * contenido) {

	//Es el desplazamiento dentro del bloque de memoria principal para luego conseguir el frame en el cual voy a escribir
	int desplazamiento = frame * frame_size;

	memcpy(memoria + desplazamiento + offset, contenido, tamanio);

}

