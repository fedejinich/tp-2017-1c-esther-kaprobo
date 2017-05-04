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



#endif /* HILOS_HILOKERNEL_H_ */
