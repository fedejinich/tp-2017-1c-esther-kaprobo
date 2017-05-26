/*
 * hiloKernel.h
 *
 *  Created on: 1/5/2017
 *      Author: utnso
 */

#ifndef HILOS_HILOKERNEL_H_
#define HILOS_HILOKERNEL_H_

#include "../Memoria.h"

void* hiloServidorKernel(void* arg);
void* hiloConexionKernel(void* socketKernel);


enum {
	PEDIDO_DE_PAGINAS,
	PEDIDO_DE_PAGINAS_OK,
	PEDIDO_DE_PAGINAS_FALLO
} mensajesKernelMemoria; //HAY QUE FIXEAR ESTO

#endif /* HILOS_HILOKERNEL_H_ */
