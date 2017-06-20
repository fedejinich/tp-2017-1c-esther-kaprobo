/*
 * operaciones.h
 *
 *  Created on: 7/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "Memoria.h"

void inicializarProceso(int* pid, int* paginasRequeridas);
void solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio);
void almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer);
void asignarPaginasAProceso(int pid, int paginasRequeridas);
void finalizarProceso(int pid);
void retardo();

#endif /* OPERACIONES_H_ */
