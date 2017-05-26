/*
 * tablaDePaginas.h
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */

#ifndef TABLADEPAGINAS_H_
#define TABLADEPAGINAS_H_

#include "Memoria.h"

typedef struct {
int frame;
int pid;
int pagina;
} t_entradaTablaDePaginas;

t_entradaTablaDePaginas* entradaTablaPointer;


void inicializarTablaDePaginas();
//t_entradaTablaDePaginas* getEntradaTablaDePaginas(int numeroDeEntrada);
void escribirEntradaTablaDePaginas(t_entradaTablaDePaginas*  entrada);

#endif /* TABLADEPAGINAS_H_ */
