/*
 * frames.h
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */

#ifndef FRAMES_H_
#define FRAMES_H_

#include "Memoria.h"



void escribirFrame(int frame, int offset, int tamanio, void * contenido);
void* leerFrame(int frame,int offset, int tamanio);
void liberarFrame(int frame);
int getFirstFrame();
void inicializarFramePointer();

#endif /* FRAMES_H_ */
