/*
 * cache.c
 *
 *  Created on: 18/6/2017
 *      Author: utnso
 */


#include "cache.h"

void inicializarCache() {
	cache = list_create();
}

void escribirCache(int pid, int pagina, void* contenido) {
	if(!estaEnCache(pid, pagina) && hayEspacioEnCache(pid)) {
		t_entradaCache* entrada = malloc(sizeof(t_entradaCache));

		entrada->pid = pid;
		entrada->pagina = pagina;
		entrada->contenido = contenido;

		list_add(cache, entrada);

		free(entrada);
	} else {
		t_entradaCache* entrada = getEntradaCache(pid, pagina);
		entrada->contenido = contenido; //esto me hace mucho ruido
	}
}

bool estaEnCache(int pid, int pagina) {
	int i;
	bool estaEnCache = false;
	for(i = 0; i < cache->elements_count && !estaEnCache; i++) {
		t_entradaCache* entrada = list_get(cache, i);
		estaEnCache = (entrada->pid == pid && entrada->pagina == pagina);
	}

	if(estaEnCache)
		log_info(logger, "Esta en cache PID: %i, Pagina: %i", pid, pagina);
	else
		log_info(logger, "No esta en cache PID: %i, Pagina: %i", pid, pagina);

	return estaEnCache;
}

bool hayEspacioEnCache(int pid) {
	bool hayEspacio = (cache->elements_count < entradas_cache) && (cantidadDeEntradasPorProceso(pid) < cache_x_proc);

	if(hayEspacio)
		log_info(logger, "Hay espacio disponible para el PID: %i", pid);
	else
		log_warning(logger, "No hay espacio disponible para el PID: %i", pid);

	return hayEspacio;
}

t_entradaCache* getEntradaCache(int pid, int pagina) {
	int i;
	if(estaEnCache(pid, pagina)) {
		for(i = 0; i < cache->elements_count; i++) {
			t_entradaCache* entrada = (t_entradaCache*) list_get(cache, i);
			if(entrada->pid == pid && entrada->pagina == pagina) {
				log_info(logger, "getEntradaCache (%i, %i)", pid, pagina);
				return entrada;
			}
		}
	}

	log_error("getEntradaCache(%i, %i) NULL", pid, pagina);
	return NULL;
}

int cantidadDeEntradasPorProceso(int pid) {
	int i;
	int cantidad = 0;
	for(i = 0; i < cache->elements_count; i++) {
		t_entradaCache* entrada = (t_entradaCache*) list_get(cache, i);
		if(entrada->pid == pid)
			cantidad++;
	}

	log_info(logger, "Cantidad de entradas en cache de PID = %i: %i", pid, cantidad);
	return cantidad;
}

void liberarProcesoDeCache(int pid) {

}

