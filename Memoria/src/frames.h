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
t_frame* framePointer;

void escribirFrame(int frame, int offset, int tamanio, void * contenido);
void liberarFrame(int frame);
int getFirstFrame();
void inicializarFramePointer();

#endif /* FRAMES_H_ */
