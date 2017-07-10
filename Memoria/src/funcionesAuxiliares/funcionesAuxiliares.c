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

int getCantidadDeEntradasPorFrame() {
	return getParteEntera(frame_size/sizeof(t_entradaTablaDePaginas));
}

int getLimiteFrameByOffset(int offset) {
	return getParteEntera(frame_size/offset) * offset;
}

int numeroDeEntradaEnFrameBy(int numeroDeEntradaTablaDePaginas) {
	if(numeroDeEntradaTablaDePaginas == 0) //Pattern matching
		return 0;
	int cantidadDeEntradasPorFrame = getCantidadDeEntradasPorFrame();
	int numeroDeEntrada = numeroDeEntradaTablaDePaginas % cantidadDeEntradasPorFrame;
	if(numeroDeEntrada == 0) //Mas pattern matching en el caso de que la entrada sea multiplo de 21
		return 21;

	return numeroDeEntrada;
}

int numeroDeFrameBy(int numeroDeEntradaTablaDePaginas) {
	if(numeroDeEntradaTablaDePaginas == 0) //Pattern matching
		return 0;
	int numeroDeFrame = getParteEntera(numeroDeEntradaTablaDePaginas/getCantidadDeEntradasPorFrame());
	if((numeroDeEntradaTablaDePaginas % getCantidadDeEntradasPorFrame()) == 0) //Mas pattern matching
		return numeroDeFrame - 1;
	return numeroDeFrame;
}

char* getCodigoDeOperacion(int codigoInt) {
	char* codigoString = malloc(50);

	if(codigoInt == INICIALIZAR_PROCESO) {
		codigoString = "INICIALIZAR PROCESO";
		return codigoString;
	} else if(codigoInt == ASIGNAR_PAGINAS) {
		codigoString = "ASIGNAR_PAGINAS";
		return codigoString;
	} else if(codigoInt == ALMACENAR_BYTES) {
		codigoString = "ALMACENAR_BYTES";
		return codigoString;
	} else if(codigoInt == SOLICITAR_BYTES) {
		codigoString = "SOLICITAR_BYTES";
		return codigoString;
	} else if(codigoInt == FINALIZAR_PROCESO) {
		codigoString = "FINALIZAR_PROCESO";
		return codigoString;
	} else if(codigoInt == LIBERAR_PAGINA) {
		codigoString = "LIBERAR_PAGINA";
		return codigoString;
	}

	codigoString = "ERROR";
	return EXIT_FAILURE_CUSTOM;
}

t_list* getCodigosParciales(char* codigo, int size) {
	t_list* codigosParciales = list_create();
	int i;
	int tamanioCodigo = strlen(codigo);
	for(i = 0; i < tamanioCodigo; i = i + size) {
		char* codigoParcial = string_substring(codigo, i, size);
		list_add(codigosParciales,codigoParcial);
	}

	return codigosParciales;
}
