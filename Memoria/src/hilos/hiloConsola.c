/*
 * hiloConsola.c
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#include "hiloConsola.h"

void* hiloConsolaMemoria() {
	inicializarVariables();
	comando = malloc(50);
	size_t size = 50;
	log_info(logger,"Inicio del hilo Consola");
	for(;;) {
		getline(&comando, &size, stdin);
		//opciones para consola memoria
		if(esRetardo(comando))
			retardoUpdate(comando);
		else if(esRetardoSolo(comando))
			log_info(logger,"Retardo de memoria: %i milisegundos.", retardo_memoria);
		else if(esDumpTabla(comando))
			dumpTabla();
		else if(esDumpCache(comando))
			dumpCache();
		else if(esDumpPID(comando))
			dumpPID(comando);
		else if(esDumpAll(comando))
			dumpAll();
		else if(esSizeMemory(comando))
			sizeMemory();
		else if(esSizePID(comando))
			sizePID(comando);
		else if(esFlush(comando))
			flush();
	}

}

void inicializarVariables() {
	retardoCommand = malloc(sizeof(string_length("retardo ")));
	retardoCommand = "retardo ";
	sizePIDCommand = malloc(sizeof(string_length("size pid ")));
	sizePIDCommand = "size pid ";
	dumpPIDCommand = malloc(sizeof(string_length("dump pid ")));
	dumpPIDCommand = "dump pid ";
}

bool esRetardo(char* comando) {
	char* posibleRetardo = string_substring(comando, 0, string_length(retardoCommand));
	return string_equals_ignore_case(posibleRetardo, retardoCommand);
}

bool esRetardoSolo(char* comando) {
	char* posibleRetardo = string_substring(comando, 0, (string_length(retardoCommand) - 1));
	return string_equals_ignore_case(posibleRetardo, string_substring(retardoCommand, 0, (string_length(retardoCommand) - 1)));
}

bool esDumpTabla(char* comando) {
	return string_equals_ignore_case(comando, "dump tabla\n");
}

bool esDumpCache(char* comando) {
	return string_equals_ignore_case(comando, "dump cache\n");
}

bool esDumpPID(char* comando) {
	char* posibleDumpPid = string_substring(comando, 0, string_length(dumpPIDCommand));
	return string_equals_ignore_case(posibleDumpPid, dumpPIDCommand);
}

bool esSizeMemory(char* comando) {
	return string_equals_ignore_case(comando, "size memory\n");
}

bool esSizePID(char* comando) {
	char* posibleSizePID = string_substring(comando, 0, string_length(sizePIDCommand));
	return string_equals_ignore_case(posibleSizePID, sizePIDCommand);
}

bool esFlush(char* comando) {
	return string_equals_ignore_case(comando,"flush\n");
}

bool esDumpAll(char* comando) {
	return string_equals_ignore_case(comando, "dump\n");
}

void retardoUpdate(char* comando) {
	char* nuevoRetardoString = string_substring(comando, string_length(retardoCommand), string_length(comando));
	if(isNumber(nuevoRetardoString)) {
		log_warning(logger, "Cambiando retardo de memoria...");
		log_warning(logger, "Retardo de memoria viejo: %i milisegundos.", retardo_memoria);
		int nuevoRetardo = atoi(nuevoRetardoString);
		retardo_memoria = nuevoRetardo;
		log_warning(logger, "Nuevo retardo de memoria: %i milisegundos.", retardo_memoria);
	} else {
		log_error(logger, "Escriba el comando nuevamente.");
	}
}

void dumpTabla() {
	//QUE ES LO DE LISTADO DE PROCESOS ACTIVOS?
	remove("dumpTabla.log");
	t_log* logDumpTabla = log_create("dumpTabla.log", "Memoria", false, LOG_LEVEL_TRACE);

	log_warning(logger, "Iniciando dump tabla...");
	log_info(logDumpTabla, "Cantidad de filas: %i", tablaDePaginasSize());

	int i;
	for(i = 0; i <= tablaDePaginasSize(); i++) {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginas(i);
		log_info(logDumpTabla, "Frame: %i, PID: %i, Pagina: %i", entrada->frame, entrada->pid, entrada->pagina);
	}

	log_warning(logger, "Dump tabla realizado correctamente.");
}

void dumpCache() {
	remove("dumpCache.log");
	t_log* logDumpCache = log_create("dumpCache.log", "Memoria", false, LOG_LEVEL_TRACE);

	log_warning(logger, "Iniciando dump cache...");
	log_info(logDumpCache, "Tamanio cache %i", entradas_cache);
	log_info(logDumpCache, "Cantidad de entradas usadas: %i", cacheCantidadEntradasUsadas());
	log_info(logDumpCache, "Cantidad de entradas libres: %i", cacheCantidadEntradasLibres());

	int i;
	for(i = 0; i < list_size(cache); i++) {
		t_entradaCache* entrada = list_get(cache, i);
		log_info(logDumpCache, "Entrada Cache. PID %i Pagina %i Contador %i", entrada->pid, entrada->pagina, entrada->cantidadDeLecturasSinUsar);
	}

	log_warning(logger, "Dump cache realizado correctamente");
}

void dumpPID(char* comando) {
	remove("dumpPID.log");
	t_log* logDumpPID = log_create("dumpPID.log", "Memoria", false, LOG_LEVEL_TRACE);

	char* pidADumpearString = string_substring(comando, string_length(dumpPIDCommand), string_length(comando));

	log_warning(logger, "Iniciando dump PID %s", pidADumpearString);

	if(isNumber(pidADumpearString)) {
		int pidADumpear = atoi(pidADumpearString);

		t_list* entradasDePID = getEntradasDePID(pidADumpear);
		int i;
		for(i = 0; i < list_size(entradasDePID); i++) {
			void* bufferDeLectura = malloc(frame_size);
			int frameADumpear = ((t_entradaTablaDePaginas *) list_get(entradasDePID, i))->frame;
			int paginaADumpear = ((t_entradaTablaDePaginas *) list_get(entradasDePID, i))->pagina;

			leerFrame(frameADumpear, 0, frame_size, bufferDeLectura);

			log_info(logDumpPID, "Pagina %i, Frame %i, Contenido \n %s", frameADumpear, paginaADumpear, bufferDeLectura);

			free(bufferDeLectura);
		}

	}

	log_debug(logger, "Dump PID %s realizado correctamente",  pidADumpearString);
}

void dumpAll() {
	remove("dumpAll.log");
	t_log* logDumpAll = log_create("dumpAll.log", "Memoria", false, LOG_LEVEL_TRACE);

	int i;
	for(i = 0; i < list_size(tablaDePaginas); i++) {
		t_entradaTablaDePaginas* entrada = list_get(tablaDePaginas, i);
		log_info(logDumpAll, " PID = %i", entrada->pid);
		if(entrada->pid != -1) {
			void* buffer = malloc(frame_size);
			leerFrame(entrada->frame, 0, frame_size, buffer);
			log_info(logDumpAll, "Frame %i, PID %i, Pagina %i, Contenido:\n %s", entrada->frame, entrada->pid, entrada->pagina, buffer);

		}
	}


}

int sizeMemory() {
	framesLibres = getCantidadFramesDisponibles();
	framesOcupados = getCantidadFramesOcupados();

	if(framesOcupados == EXIT_FAILURE_CUSTOM || framesLibres == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error en sizeMemory()");
		return EXIT_FAILURE_CUSTOM;
	}

	log_warning(logger, "Cantidad de frames: %i, frames libres: %i, frames ocupados: %i", frames, framesLibres, framesOcupados);
}

void sizePID(char* comando) {
	char* PIDString = string_substring(comando, string_length(sizePIDCommand), string_length(comando));
	if(isNumber(PIDString)) {
		int pid = atoi(PIDString);
		t_list* entradasPID = getEntradasDePID(pid);
		int cantidadDePaginas = entradasPID->elements_count;

		log_warning(logger, "PID %i tiene un tamanio de %i paginas en memoria", cantidadDePaginas);

		list_destroy(entradasPID);
	} else {
		log_error(logger, "Escriba el comando nuevamente.");
	}
}


void flush() {
	log_warning(logger, "Flushing cache...");
	list_destroy(cache);
	cache = list_create();
	log_warning(logger, "Flush completo");
}

bool isNumber(char* palabra) {

	int tamanio = string_length(palabra)-1;

	t_list * lista_chars = list_create();

	int i;
	for (i = 0; i < tamanio; i++) {

		list_add(lista_chars, (void *) palabra[i]);

	}

	bool es_digito(void * elemento) {

		char * letra = (char *) elemento;

		return isdigit(letra);
	}

	bool resultado = list_all_satisfy(lista_chars, es_digito);

	free(lista_chars);

	return resultado;

}


