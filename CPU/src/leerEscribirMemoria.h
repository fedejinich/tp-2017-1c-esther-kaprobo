/*
 * leerEscribirMemoria.h
 *
 *  Created on: 2/7/2017
 *      Author: utnso
 */

#ifndef LEERESCRIBIRMEMORIA_H_
#define LEERESCRIBIRMEMORIA_H_

#include "CPU.h"

void almacenarBytesEnMemoria(t_direccion* direccion, void* buffer);
void* solicitarBytesAMemoria(t_direccion* direccion) ;



#endif /* LEERESCRIBIRMEMORIA_H_ */
