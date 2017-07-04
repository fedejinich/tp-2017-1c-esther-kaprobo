/*
 * operaciones.h
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "../Memoria.h"


int inicializarProceso(int pid, int paginasRequeridas);
int asignarPaginasAProceso(int pid, int paginasAsignar);
int finalizarProceso(int pid);
int liberarPaginaProceso(int pid, int pagina);
int almacenarCodigo(int pid, int paginasCodigo, char* codigo);

#endif /* OPERACIONES_H_ */
