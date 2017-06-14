/*
 * frames.h
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */

#ifndef FRAMES_H_
#define FRAMES_H_

#include "Memoria.h"

typedef struct {
	char data[256];
} t_frame;

int framesLibres;
int framesOcupados;

t_frame* memoria;

void escribir_frame(int frame, int offset, int tamanio, void * contenido);

#endif /* FRAMES_H_ */
