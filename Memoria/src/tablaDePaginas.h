/*
 * tablaDePaginas.h
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */

#ifndef TABLADEPAGINAS_H_
#define TABLADEPAGINAS_H_

#include "Memoria.h"



t_entradaTablaDePaginas* tablaDePaginas;



void inicializarTablaDePaginas();
int escribirTablaDePaginas(int frame, int pid, int pagina);
int getFrameDisponible();
int tablaDePaginasSize();
int getCantidadFramesDisponibles();
int getCantidadFramesOcupados();
bool paginasDisponibles(int pid, int paginasRequeridas);
t_entradaTablaDePaginas* getEntradaTablaDePaginas(int numeroDeEntrada);
int reservarPaginas(int pid, int paginasAReservar);
int asignarMasPaginasAProceso(int pid, int paginasAsignar);
int getUltimaPagina(int pid);
t_list* getEntradasDePID(int pid);
int esPaginaLiberable(int pid, int pagina);
int liberarPagina(int pid, int pagina);
int getFrameByPIDPagina(int pid, int pagina);
bool existePIDPagina(int pid, int pagina);
int getFramePrimeraPagina(int pid);
int getTablaDePaginasBytes();


#endif /* TABLADEPAGINAS_H_ */
