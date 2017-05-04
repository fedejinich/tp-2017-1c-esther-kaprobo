/*
 * hiloCPU.h
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#ifndef HILOS_HILOCPU_H_
#define HILOS_HILOCPU_H_

#include "../Memoria.h"

void* hiloServidorCPU(void* arg);
void* hiloConexionCPU(void* socket);


#endif /* HILOS_HILOCPU_H_ */
