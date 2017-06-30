/*
 *retardo operacionesMemoriaCPU.h
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */

#ifndef OPERACIONESMEMORIACPU_H_
#define OPERACIONESMEMORIACPU_H_

#include "../Memoria.h"

int almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer);

#endif /* OPERACIONESMEMORIACPU_H_ */
