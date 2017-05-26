/*
 * tablaDePaginas.h
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */

#ifndef TABLADEPAGINAS_H_
#define TABLADEPAGINAS_H_

#include "Memoria.h"
#include "frames.h"

typedef struct {
	int frame;
	int pid;
	int pagina;
} t_entradaTablaDePaginas;

t_entradaTablaDePaginas* entradaTablaPointer;


void inicializarTablaDePaginas();
//t_entradaTablaDePaginas* getEntradaTablaDePaginas(int numeroDeEntrada);
void escribirTablaDePaginas(t_entradaTablaDePaginas*  entrada);
bool espacioDisponible(int paginasRequeridas, int tamanioCodigo);
int getFrameDisponible();

#endif /* TABLADEPAGINAS_H_ */
