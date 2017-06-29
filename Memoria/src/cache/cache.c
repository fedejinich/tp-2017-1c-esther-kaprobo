/*
 * cache.c
 *
 *  Created on: 18/6/2017
 *      Author: utnso
 */


#include "cache.h"

int inicializarCache() {
	log_info(logger, "Inicializando cache");
	cache = -1;
	cache = list_create();

	if(cache == -1) {
		log_error(logger, "Error al inicializar cache");
		return EXIT_FAILURE;
	}
	log_debug(logger, "Cache inicializada");
	return EXIT_SUCCESS;
}

int escribirCache(int pid, int pagina, void* contenido) {
	log_info(logger, "Escribiendo cache PID %i Pagina %i", pid, pagina);
	t_entradaCache* entrada = malloc(sizeof(t_entradaCache));
	int exito = incrementarCantidadDeLecturas();
	if(exito == EXIT_FAILURE) {
		log_error(logger, "Error al escribir cache");
		return EXIT_FAILURE;
	}

	if(hayEspacioEnCache(pid)) {
		entrada->pid = pid;
		entrada->pagina = pagina;
		entrada->cantidadDeLecturasSinUsar = 0;
		entrada->contenido = contenido;

		list_add(cache, entrada);
	} else {
		//ACA ESTA LA GRACIA DEL LRU
		log_warning(logger, "No hay mas espacio en memoria cache, remplazando elementos");

		int exito = remplazoLRU(pid, pagina, contenido);

		if(exito == EXIT_FAILURE) {
			log_error(logger, "No se pudo escribir en cache, PID %i, Pagina %i", pid, pagina);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

void* leerDeCache(int pid, int pagina) {
	incrementarCantidadDeLecturas();
	t_entradaCache* entrada = getEntradaCache(pid, pagina);

	if(entrada == EXIT_FAILURE) {
		log_warning(logger, "No se pudo leer de cache PID %i Pagina %i", pid, pagina);
		return EXIT_FAILURE;
	}

	entrada->cantidadDeLecturasSinUsar = 0;
	log_debug(logger, "Leido de cache PID %i, pagina %i", pid, pagina);

	return entrada->contenido;
}

int liberarProcesoDeCache(int pid) {
	int i;
	for(i = 0; i <= cache->elements_count; i++) {
		t_entradaCache* entrada = list_get(cache, i);
		if(entrada->pid == pid) {
			log_warning(logger, "Liberando contenido de cache PID: %i, Pagina: %i", entrada->pid, entrada->pagina);
			list_remove(cache,i);
			i = 0;
			log_warning(logger, "Liberado contenido de cache PID: %i, Pagina: %i");
		}
	}
	log_debug(logger, "Liberado todo el contenido del PID %i de cache", pid);
	return EXIT_SUCCESS;
}

int remplazoLRU(int pid, int pagina, void* contenido) {
	log_info(logger, "Usando algoritmo %s para remplazo de elementos en memoria cache", reemplazo_cache);
	t_entradaCache* entradaMasAntigua = getEntradaMasAntigua(pid);


	if(entradaMasAntigua == EXIT_FAILURE) {
		log_error(logger, "Error en remplazo LRU, PID %i, Pagina %i", pid, pagina);
		return EXIT_FAILURE;
	}

	log_info(logger, "Remplazando entrada cache. PID %i, Pagina %i",
			entradaMasAntigua->pid, entradaMasAntigua->pagina, entradaMasAntigua->cantidadDeLecturasSinUsar);

	entradaMasAntigua->pid = pid;
	entradaMasAntigua->pagina = pagina;
	entradaMasAntigua->cantidadDeLecturasSinUsar = 0;
	entradaMasAntigua->contenido = contenido;

	log_info(logger, "Remplazada entrada cache. PID %i, Pagina %i",
			entradaMasAntigua->pid, entradaMasAntigua->pagina, entradaMasAntigua->cantidadDeLecturasSinUsar);

	log_debug(logger, "Se escribio en cache PID %i, Pagina %i", entradaMasAntigua->pid, entradaMasAntigua->pagina); //este log deberia estar en escribirCache
	//pero por un tema de simplicidad lo meti aca
	return EXIT_SUCCESS;
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

	log_error("No se pudo encontrar la entrada para el PID %i, Pagina %i", pid, pagina);
	return EXIT_FAILURE;
}

t_entradaCache* getEntradaMasAntigua(int pid) {
	t_entradaCache* entradaAux;
	t_entradaCache* entradaMasAntigua = malloc(sizeof(t_entradaCache));
	entradaMasAntigua->pid = -1;
	entradaMasAntigua->pagina = -1;
	entradaMasAntigua->cantidadDeLecturasSinUsar = -1;

	if(cantidadDeEntradasPorProceso(pid) == cache_x_proc) {
		int j;
		for(j = 0; j < list_size(cache); j++) {
			entradaAux = list_get(cache, j);
			if((entradaAux->cantidadDeLecturasSinUsar >= entradaMasAntigua->cantidadDeLecturasSinUsar) && entradaAux->pid == pid)
					entradaMasAntigua = entradaAux;
		}
	} else {
		int i;
		for(i = 0; i < list_size(cache); i++) {
			entradaAux = list_get(cache, i);
			if((entradaAux->cantidadDeLecturasSinUsar >= entradaMasAntigua->cantidadDeLecturasSinUsar))
					entradaMasAntigua = entradaAux;
		}
	}

	if(entradaMasAntigua->pid == -1) {
		log_error(logger, "Error al buscar entrada mas antigua");
		return EXIT_FAILURE;
	}

	return entradaMasAntigua;
}

bool estaEnCache(int pid, int pagina) {
	if(list_size(cache) > 0) {
		int i;
		bool estaEnCache = false;
		for(i = 0; i <= list_size(cache) && !estaEnCache; i++) {
			t_entradaCache* entrada = list_get(cache, i);
			estaEnCache = (entrada->pid == pid && entrada->pagina == pagina);
		}

		if(estaEnCache)
			log_info(logger, "Esta en cache PID: %i, Pagina: %i", pid, pagina);
		else
			log_info(logger, "No esta en cache PID: %i, Pagina: %i", pid, pagina);

		return estaEnCache;
	}
	log_info(logger, "No esta en cache PID: %i, Pagina: %i", pid, pagina);

	return false;
}

bool hayEspacioEnCache(int pid) {
	int cantidadEntradasPID = cantidadDeEntradasPorProceso(pid);
	bool hayEspacio = (cache->elements_count < entradas_cache) && (cantidadEntradasPID < cache_x_proc); //es <= ?

	if(!hayEspacio)
		log_warning(logger, "No hay espacio disponible para el PID %i", pid);

	return hayEspacio;
}

int cantidadDeEntradasPorProceso(int pid) {
	int i;
	int cantidad = 0;

	for(i = 0; i < cache->elements_count; i++) {
		t_entradaCache* entrada = (t_entradaCache*) list_get(cache, i);
		if(entrada->pid == pid)
			cantidad++;
	}

	return cantidad;
}

int incrementarCantidadDeLecturas() {
	if(!list_is_empty(cache)) {
		int i;
		for(i = 0; i < list_size(cache); i++) {
			t_entradaCache* entrada = (t_entradaCache*) list_get(cache, i);
			//despues saca los warnings que son super al pedo
			entrada->cantidadDeLecturasSinUsar++;
		}
	}
	return EXIT_SUCCESS;
}

bool superaElLimitePorPID(int pid) {
	return cantidadDeEntradasPorProceso(pid) <= cache_x_proc ;
}

int cacheCantidadEntradasUsadas() {
	return list_size(cache);
}

int cacheCantidadEntradasLibres() {
	return entradas_cache - list_size(cache);
}
