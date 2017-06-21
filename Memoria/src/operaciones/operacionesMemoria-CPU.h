/*
 *retardo operacionesMemoriaCPU.h
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONESMEMORIACPU_H_
#define OPERACIONESMEMORIACPU_H_

#include "../Memoria.h"

bool almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer);
void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio);

#endif /* OPERACIONESMEMORIACPU_H_ */
