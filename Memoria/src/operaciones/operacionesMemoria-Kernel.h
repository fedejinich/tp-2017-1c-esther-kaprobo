/*
 * operaciones.h
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "../Memoria.h"


void inicializarProceso(int* pid, int* paginasRequeridas);
void asignarPaginasAProceso(int pid, int paginasRequeridas);
void finalizarProceso(int pid);

#endif /* OPERACIONES_H_ */
