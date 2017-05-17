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
	else
		return parteEntera;
}
