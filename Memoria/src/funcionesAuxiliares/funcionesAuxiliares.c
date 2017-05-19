/*
 * funcionesAuxiliares.c
 *
 *  Created on: 17/5/2017
 *      Author: utnso
 */

#include "funcionesAuxiliares.h"

int getCantidadDeFramesTablaDePaginas() {
	double cantidad = (frames*sizeof(t_entradaTablaDePaginas))/frame_size;
	int parteEntera = getParteEntera(cantidad);
	double parteDecimal = getParteDecimal(cantidad);

	if(parteDecimal > 0)
		return parteEntera + 1;

	return parteEntera;
}



int getLimiteFrameByOffset(int offset) {
	return getParteEntera(frame_size/offset) * offset;
}
