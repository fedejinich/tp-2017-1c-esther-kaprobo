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

typedef struct {
	int pid;
	int paginasAPedir;
} t_pedidoDePaginas; //DESPUES HAY QUE HACER UN FIX DE ESTO Y DEFINIR ESTE STRUCT SOLO EN ESTRUCTURAS.H
//PERO AHORA EL PUTO DE C NO C PORQUE NO ME ESTA DEJANDO

void inicializarTablaDePaginas();
//t_entradaTablaDePaginas* getEntradaTablaDePaginas(int numeroDeEntrada);
void escribirTablaDePaginas(t_entradaTablaDePaginas*  entrada);
bool espacioDisponible(int paginasRequeridas, int tamanioCodigo);
int getFrameDisponible();

#endif /* TABLADEPAGINAS_H_ */
