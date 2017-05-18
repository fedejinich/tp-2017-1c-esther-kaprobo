/*
 * funcionesAuxiliares.c
 *
 *  Created on: 17/5/2017
 *      Author: utnso
 */

#include "funcionesAuxiliares.h"

int getCantidadDeMarcosTablaDePaginas() {
	double cantidad = (marcos*sizeof(t_entradaTablaDePaginas))/marco_size;
	int parteEntera = (int) cantidad;
	double parteDecimal = cantidad - parteEntera;

	if(parteDecimal > 0)
		return parteEntera + 1;

	return parteEntera;
}



int getLimiteMarcoByOffset(int offset) {
	int limite = marco_size - offset; //hay que agregarle un -1 aca?

	if(offset >= marco_size)
		return -1;	//ojala se puediese tirar una excepcion

	return limite;
}
