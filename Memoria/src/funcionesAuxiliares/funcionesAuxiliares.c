/*
 * funcionesAuxiliares.c
 *
 *  Created on: 17/5/2017
 *      Author: utnso
 */

#include "funcionesAuxiliares.h"

int getCantidadDeMarcosTablaDePaginas() {
	double cantidad = (marcos*sizeof(t_entradaTablaDePaginas))/marco_size;
	int parteEntera = getParteEntera(cantidad);
	double parteDecimal = getParteDecimal(cantidad);

	if(parteDecimal > 0)
		return parteEntera + 1;

	return parteEntera;
}



int getLimiteMarcoByOffset(int offset) {
	return getParteEntera(marco_size/offset) * offset;
}
