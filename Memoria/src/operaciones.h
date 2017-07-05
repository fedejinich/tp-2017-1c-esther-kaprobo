/*
 * operaciones.h
 *
 *  Created on: 5/7/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_OPERACIONES_H_
#define OPERACIONES_OPERACIONES_H_

#include "Memoria.h"

void retardo();
void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio);
int almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer);
int inicializarProceso(int pid, int paginasRequeridas);
int asignarPaginasAProceso(int pid, int paginasAsignar);
int finalizarProceso(int pid);
int liberarPaginaProceso(int pid, int pagina);
int almacenarCodigo(int pid, int paginasCodigo, char* codigo);

#endif /* OPERACIONES_OPERACIONES_H_ */
