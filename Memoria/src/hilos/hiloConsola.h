/*
 * hiloConsola.h
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#ifndef HILOS_HILOCONSOLA_H_
#define HILOS_HILOCONSOLA_H_

#include "../Memoria.h"

void* hiloConsolaMemoria();

char comando[50];

void flush();
void size();
void retardo(int retardo);
void dump();

#endif /* HILOS_HILOCONSOLA_H_ */
