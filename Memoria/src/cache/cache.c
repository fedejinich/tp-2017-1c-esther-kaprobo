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
		return EXIT_FAILURE_CUSTOM;
	}
	log_debug(logger, "Cache inicializada");
	return EXIT_SUCCESS_CUSTOM;
}

int escribirCache(int pid, int pagina, int tamanio, void* contenido) {
	if(estaEnCache(pid, pagina)) {
		log_warning(logger, "Actualizando en Cache PID %i Pagina %i", pid, pagina);
		t_entradaCache* entrada = getEntradaCache(pid, pagina);
		entrada->contenido = contenido;

		log_debug(logger, "Actualizado en Cache PID %i, Pagina %i", pid, pagina);
		return EXIT_SUCCESS_CUSTOM;
	}

	log_info(logger, "Escribiendo cache PID %i Pagina %i", pid, pagina);

	t_entradaCache* entrada = malloc(sizeof(t_entradaCache));

	int exito = incrementarCantidadDeLecturas();

	if(exito == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error al escribir cache");
		return EXIT_FAILURE_CUSTOM;
	}

	if(hayEspacioEnCache(pid, pagina)) {
		entrada->pid = pid;
		entrada->pagina = pagina;
		entrada->cantidadDeLecturasSinUsar = 0;
		entrada->contenido = contenido;

		list_add(cache, entrada);
	} else {
		if(cache_x_proc > 0) {
			//ACA ESTA LA GRACIA DEL LRU
			log_warning(logger, "No hay mas espacio en memoria cache, remplazando elementos");

			int exito = remplazoLRU(pid, pagina, contenido);

			if(exito == EXIT_FAILURE_CUSTOM) {
				log_error(logger, "No se pudo escribir en cache, PID %i, Pagina %i", pid, pagina);
				return EXIT_FAILURE_CUSTOM;
			}
		}
	}

	log_debug(logger, "Se escribio en cache PID %i, Pagina %i", pid, pagina);
	return EXIT_SUCCESS_CUSTOM;
}

void* leerDeCache(int pid, int pagina, int offset, int tamanio) {
	log_info(logger, "Solicitando bytes de cache. PID %i, Pagina %i ...", pid, pagina);

	incrementarCantidadDeLecturas();
	t_entradaCache* entrada = getEntradaCache(pid, pagina);

	if(entrada == EXIT_FAILURE_CUSTOM) {
		log_warning(logger, "No se pudo completar la solicitud de bytes de cache. PID %i Pagina %i", pid, pagina);
		return EXIT_FAILURE_CUSTOM;
	}

	entrada->cantidadDeLecturasSinUsar = 0;
	log_debug(logger, "Solicitud de bytes de cache exitosa. PID %i Pagina %i", pid, pagina);

	void* buffer = malloc(tamanio + 1); //por el \0
	memcpy(buffer, entrada->contenido + offset, tamanio);
	strcpy(buffer + tamanio, "\0");

	return buffer;
}

bool estaEnCachePID(int pid) {
	if(list_size(cache) <= 0) {
		log_info("No hay elementos en cache", pid);
		return false;
	}

	bool esta = false;
	int i;
	for(i = 0; i < list_size(cache); i++) {
		t_entradaCache* entrada = list_get(cache, i);
		if(entrada->pid == pid) {
			log_info(logger, "PID %i se encuentra en cache", pid);
			return true;
		}
	}

	log_info(logger, "PID %i no se encuentra en cache", pid);
	return false;
}

int liberarProcesoDeCache(int pid) {
	log_warning(logger, "Liberando de cache PID %i", pid);
	bool existeProceso = estaEnCachePID(pid);

	while(existeProceso) {
		int i;
		for(i = 0; i < list_size(cache) && existeProceso; i++) {
			t_entradaCache* entrada = list_get(cache, i);
			if(entrada->pid == pid) {
				log_warning(logger, "Liberando contenido de cache PID: %i, Pagina: %i", entrada->pid, entrada->pagina);
				list_remove(cache,i);
				log_warning(logger, "Liberado contenido de cache PID: %i, Pagina: %i");
				existeProceso = estaEnCachePID(pid);
			}
		}
	}

	if(!existeProceso) {
		log_warning(logger, "No existe en cache PID %i");
		log_warning(logger, "No se libero de cache PID %i");
		return EXIT_SUCCESS_CUSTOM;
	}

	log_debug(logger, "Liberado todo el contenido del PID %i de cache", pid);
	return EXIT_SUCCESS_CUSTOM;
}

int remplazoLRU(int pid, int pagina, void* contenido) {
	log_info(logger, "Usando algoritmo %s para remplazo de elementos en memoria cache", reemplazo_cache);
	t_entradaCache* entradaMasAntigua = getEntradaMasAntigua(pid);


	if(entradaMasAntigua == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error en remplazo LRU, PID %i, Pagina %i", pid, pagina);
		return EXIT_FAILURE_CUSTOM;
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
	return EXIT_SUCCESS_CUSTOM;
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
	return EXIT_FAILURE_CUSTOM;
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
		return EXIT_FAILURE_CUSTOM;
	}

	return entradaMasAntigua;
}

bool estaEnCache(int pid, int pagina) {
	//printf("CANTIDAD DE ELEMENTOS EN CACHE %i\n", list_size(cache));
	//printf("PID a cheuear %i Pagina %i\n", pid, pagina);
	if(list_size(cache) > 0) {
		int i;
		bool estaEnCache = false;
		for(i = 0; i < list_size(cache) && !estaEnCache; i++) {
			//printf("list_get(cache,%i)\n", i);
			t_entradaCache* entrada = list_get(cache, i);
			//printf("despues del list_get\n");
			//printf("Entrada cache PID %i, Pagina %i", entrada->pid, entrada->pagina);
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

bool hayEspacioEnCache(int pid, int pagina) {
	int cantidadEntradasPID = cantidadDeEntradasPorProceso(pid);
	log_warning(logger, "hayEspacio = %i < %i && %i < %i", cache->elements_count, entradas_cache, cantidadEntradasPID, cache_x_proc);
	bool hayEspacio = (cache->elements_count < entradas_cache) && (cantidadEntradasPID < cache_x_proc); //es <= ?

	log_info(logger, "Cantidad de elementos en cache: %i", cache->elements_count);
	log_info(logger, "Cantidad de elementos de PID %i: %i", cantidadEntradasPID);

	if(!hayEspacio) {
		log_warning(logger, "No hay espacio disponible en cache para el PID %i, Pagina %i", pid, pagina);
		return hayEspacio;
	}

	log_info(logger, "Hay espacio en cache para PID %i, Pagina %i", pid, pagina);
	return hayEspacio;
}

int cantidadDeEntradasPorProceso(int pid) {
	int i;
	int cantidad = 0;
	int cantidadDeElementosEnCache = list_size(cache);

	if(cantidadDeElementosEnCache == 0) {
		return cantidad;
	}

	for(i = 0; i < cantidadDeElementosEnCache; i++) {
		t_entradaCache* entrada = (t_entradaCache*) list_get(cache, i);
		log_warning(logger, "Entrada cache %i:  PID %i, Pagina %i, Lecturas Sin Usar %i", i, entrada->pid, entrada->pagina, entrada->cantidadDeLecturasSinUsar);
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
	return EXIT_SUCCESS_CUSTOM;
}

bool superaElLimitePorPID(int pid) {
	return cantidadDeEntradasPorProceso(pid) <= cache_x_proc ;
}

int cacheCantidadEntradasUsadas() {
	return list_size(cache);
}

void bloquearCache() {
	pthread_mutex_lock(&cacheMutex);
	log_info(logger, "Bloqueo cache");
}

void desbloquearCache() {
	pthread_mutex_unlock(&cacheMutex);
	log_info(logger, "Desbloqueo cache");
}

int cacheCantidadEntradasLibres() {
	return entradas_cache - list_size(cache);
}


