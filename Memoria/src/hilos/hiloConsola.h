/*
 * hiloConsola.h
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#ifndef HILOS_HILOCONSOLA_H_
#define HILOS_HILOCONSOLA_H_

#include "../Memoria.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

void* hiloConsolaMemoria();
void inicializarVariables();
char* comando;

bool esFlush(char* comando);
bool esDump(char* comando);
bool esSize(char* comando);
bool esRetardo(char* comando);
bool esRetardoSolo(char* comando);

char* retardoCommand;

void flush();
void size();
void retardo(char* comando);
void dump();

bool isNumber(char* palabra);

#endif /* HILOS_HILOCONSOLA_H_ */
