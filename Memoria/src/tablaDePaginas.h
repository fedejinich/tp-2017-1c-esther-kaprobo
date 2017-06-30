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

t_entradaTablaDePaginas* tablaDePaginas;



void inicializarTablaDePaginas();
int escribirTablaDePaginas(int frame, int pid, int pagina);
int getFrameDisponible();
int tablaDePaginasSize();
int getCantidadFramesDisponibles();
int getCantidadFramesOcupados();
bool paginasDisponibles(int paginasRequeridas);
t_entradaTablaDePaginas* getEntradaTablaDePaginas(int numeroDeEntrada);
void reservarPaginas(int pid, int paginasAReservar);
int asignarMasPaginasAProceso(int pid, int paginasAsignar);
int getUltimaPagina(int pid);
t_list* getEntradasDePID(int pid);
int esPaginaLiberable(int pid, int pagina);
int liberarPagina(int pid, int pagina);
int getFrameByPIDPagina(int pid, int pagina);
bool existePIDPagina(int pid, int pagina);

#endif /* TABLADEPAGINAS_H_ */
