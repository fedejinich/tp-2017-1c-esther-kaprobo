/*
 * cache.h
 *
 *  Created on: 18/6/2017
 *      Author: utnso
 */

#ifndef CACHE_H_
#define CACHE_H_

#include "../Memoria.h"

typedef struct {
	int pid;
	int pagina;
	int cantidadDeLecturasSinUsar; //Indica la cantidad de veces que se ingreso a la cache y no se uso este elemento
	void* contenido;
} t_entradaCache;



t_list* cache;

int inicializarCache();
int escribirCache(int pid, int pagina, void* contenido);
bool estaEnCache(int pid, int pagina);
t_entradaCache* getEntradaCache(int pid, int pagina);
bool hayEspacioEnCache(int pid);
int cantidadDeEntradasPorProceso(int pid);
int liberarProcesoDeCache(int pid);
int liberarPaginaDeProcesoDeCache(int pid, int pagina);
void* leerDeCache(int pid, int pagina);
int incrementarCantidadDeLecturas();
int remplazoLRU(int pid, int pagina, void* contenido);
t_entradaCache* getEntradaMasAntigua(int pid);
bool superaElLimitePorPID();
int cacheCantidadEntradasUsadas();
int cacheCantidadEntradasLibres();

//QUE PASA SI HAY HUECOS?

#endif /* CACHE_H_ */
