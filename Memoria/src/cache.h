/*
 * cache.h
 *
 *  Created on: 18/6/2017
 *      Author: utnso
 */

#ifndef CACHE_H_
#define CACHE_H_

#include "Memoria.h"

typedef struct {
	int pid;
	int pagina;
	void* contenido;
} t_entradaCache;

t_list* cache;

void inicializarCache();
void escribirCache(int pid, int pagina, void* contenido);
bool estaEnCache(int pid, int pagina);
t_entradaCache* getEntradaCache(int pid, int pagina);
bool hayEspacioEnCache(int pid);
int cantidadDeEntradasPorProceso(int pid);
void liberarProcesoDeCache(int pid);
void liberarPaginaDeProcesoDeCache(int pid, int pagina);
#endif /* CACHE_H_ */
