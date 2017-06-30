/*
 * abstractOperaciones.h
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONES_ABSTRACTOPERACIONES_H_
#define OPERACIONES_ABSTRACTOPERACIONES_H_

#include "../Memoria.h"

void retardo();
void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio);

#endif /* OPERACIONES_ABSTRACTOPERACIONES_H_ */
