/*
 * cache.c
 *
 *  Created on: 18/6/2017
 *      Author: utnso
 */


#include "cache.h"

int inicializarCache() {
	cache = -1;
	cache = list_create();

	if(cache == -1) {
		log_error(logger, "Error al inicializar cache");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int escribirCache(int pid, int pagina, void* contenido) {
	int exito = incrementarCantidadDeLecturas();

	if(exito == EXIT_FAILURE) {
		log_error(logger, "Error al escribir cache");
		return EXIT_FAILURE;
	}

	if(hayEspacioEnCache(pid)) {
		t_entradaCache* entrada = malloc(sizeof(t_entradaCache));

		entrada->pid = pid;
		entrada->pagina = pagina;
		entrada->cantidadDeLecturasSinUsar = 0;
		entrada->contenido = contenido;

		list_add(cache, entrada);

		free(entrada);
	} else {
		//ACA ESTA LA GRACIA DEL LRU
		log_warning(logger, "No hay mas espacio en memoria cache, remplazando elementos");

		int exito = remplazoLRU(pid, pagina, contenido);

		if(exito == EXIT_FAILURE) {
			log_error(logger, "No se pudo escribir en cache, PID %i, Pagina %i", pid, pagina);
			return EXIT_FAILURE;
		}
	}

	log_debug(logger, "Se escribio en cache PID %i, Pagina %i", pid, pagina);
	return EXIT_SUCCESS;
}

int remplazoLRU(int pid, int pagina, void* contenido) {
	log_info(logger, "Usando algoritmo %s para remplazo de elementos en memoria cache", reemplazo_cache);
	t_entradaCache* entrada = getEntradaMasAntigua();

	if(entrada == EXIT_FAILURE) {
		log_error(logger, "Error en remplazo LRU, PID %i, Pagina %i", pid, pagina);
		return EXIT_FAILURE;
	}

	log_info(logger, "Remplazando entrada cache. PID %i, Pagina %i");

	entrada->pid = pid;
	entrada->pagina = pagina;
	entrada->cantidadDeLecturasSinUsar = 0;
	entrada->contenido = contenido;

	log_info(logger, "Remplazada entrada cache. PID %i, Pagina %i");
	return EXIT_SUCCESS;
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

int liberarProcesoDeCache(int pid) {
	int i;
	for(i = 0; i <= cache->elements_count; i++) {
		t_entradaCache* entrada = list_get(cache, i);
		if(entrada->pid == pid) {
			log_warning(logger, "Liberando contenido de cache PID: %i, Pagina: %i", entrada->pid, entrada->pagina);
			list_remove(cache,i);
			log_warning(logger, "Liberado contenido de cache PID: %i, Pagina: %i");
		}
	}
	log_debug(logger, "Liberado todo el contenido del PID %i de cache", pid);
	return EXIT_SUCCESS;
}

void* leerDeCache(int pid, int pagina) {
	incrementarCantidadDeLecturas();
	t_entradaCache* entrada = getEntradaCache(pid, pagina);
	entrada->cantidadDeLecturasSinUsar = 0;
	log_debug(logger, "Leido de cache PID %i, pagina %i", pid, pagina);

	return entrada->contenido;
}

int incrementarCantidadDeLecturas() {
	//EN LA ESTRUCTURA TIPO LIST NO HAY HUECOS, SI ELIMINO UN ELEMENTO SE COMPACTA TODO
	int i;
	for(i = 0; i <= cache->elements_count; i++) {
		t_entradaCache* entrada = list_get(cache, i);
		//despues saca los warnings que son super al pedo
		log_warning(logger, "Entrada Cache, PID %i, Pagina %i, Contador %i", entrada->pid, entrada->pagina, entrada->cantidadDeLecturasSinUsar);
		entrada->cantidadDeLecturasSinUsar++;
		log_warning(logger, "Entrada Cache, PID %i, Pagina %i, Contador %i", entrada->pid, entrada->pagina, entrada->cantidadDeLecturasSinUsar);
	}

	return EXIT_SUCCESS;
}

int getEntradaMasAntigua() {
	t_entradaCache* entrada;
	t_entradaCache* entradaMasAntigua = malloc(sizeof(t_entradaCache));
	entradaMasAntigua->pid = -1;
	entradaMasAntigua->pagina = -1;
	entradaMasAntigua->cantidadDeLecturasSinUsar = -1;

	int i;
	for(i = 0; i <= cache->elements_count; i++) {
		entrada = list_get(cache, i);
		if(entrada->cantidadDeLecturasSinUsar >= entrada->cantidadDeLecturasSinUsar) {
			entradaMasAntigua = entrada;
		}
	}

	if(entrada->pid == -1) {
		log_error(logger, "Error al buscar entrada mas antigua");
		return EXIT_FAILURE;
	}

	free(entradaMasAntigua);

	log_info(logger, "Entrada mas antigua: PID %i, Pagina %i, Cantidad de lecutras sin usar %i", entrada->pid, entrada->pagina, entrada->cantidadDeLecturasSinUsar);
	return EXIT_SUCCESS;
}
