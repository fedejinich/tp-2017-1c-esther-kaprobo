/*
 * operaciones.h
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "../Memoria.h"


void inicializarProceso(int pid, int paginasRequeridas);
void asignarPaginasAProceso(int pid, int paginasAsignar);
void finalizarProceso(int pid);
void liberarPaginaProceso(int pid, int pagina);

#endif /* OPERACIONES_H_ */
