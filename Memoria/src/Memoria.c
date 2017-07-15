/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "Memoria.h"


int main(int argc, char **argv) {
	logger = iniciarLog("memoria.log","Memoria");

	printf("%s", "\n====== INICIO MEMORIA ======\n\n");

	cargarConfiguracion();
	grandMalloc();
	inicializarTablaDePaginas();
	inicializarFramePointer();
	inicializarCache();
	iniciarHilos();

	return EXIT_SUCCESS_CUSTOM;
}

void cargarConfiguracion(){
	log_info(logger,"Cargando configuracion");

	t_config* config = config_create(getenv("archivo_configuracion_memoria"));
	puerto = config_get_int_value(config, "PUERTO");
	log_info(logger,"PUERTO: %i ", puerto);

	frames = config_get_int_value(config, "MARCOS");
	log_info(logger,"CANTIDAD MARCOS: %i ", frames);

	frame_size = config_get_int_value(config, "MARCO_SIZE");
	tamanioPagina = frame_size;
	log_info(logger,"TAMAÑO MARCO: %i ", frame_size);

	entradas_cache = config_get_int_value(config, "ENTRADAS_CACHE");
	log_info(logger,"ENTRADAS CACHE DISPONIBLES: %i ", entradas_cache);

	cache_x_proc = config_get_int_value(config, "CACHE_X_PROC");
	log_info(logger,"ENTRADAS MAXIMAS POR PROCESO: %i ", cache_x_proc);

	reemplazo_cache = config_get_string_value(config, "REEMPLAZO_CACHE");
	log_info(logger,"ALGORITMO REEMPLAZO EN CACHE: %s ", reemplazo_cache);

	retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	log_info(logger,"RETARDO MEMORIA: %i ", retardo_memoria);

	log_info(logger,"El archivo de configuracion fue cargado con exito");
}

void grandMalloc() { //aca voy a reservar el bloque de memoria contiuna y crear mi tabla de paginas
	log_info(logger,"Reservando bloque de memoria contigua...");

	tamanioMemoria = (frames * frame_size);
	memoria = malloc(tamanioMemoria);

	memset(memoria, '\0', tamanioMemoria);

	log_info(logger,"Memoria continua reservada correctamente");
}



void iniciarHilos() {
	log_info(logger, "Inicializando hilos...");

	pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	pthread_create(&consolaMemoria, NULL, hiloConsolaMemoria, NULL);

	pthread_join(servidorConexionesCPU, NULL);
	pthread_join(servidorConexionesKernel, NULL);
	pthread_join(consolaMemoria, NULL);
}



void testFuncionHashObtengoPosicionCandidataOk() {
	CANTIDAD_DE_MARCOS = 10;
	inicializarOverflow(CANTIDAD_DE_MARCOS);
	escribirTablaDePaginas(5, 5, 5);

	int pid=5;
	int pagina=5;

	/* Obtengo el numero de frame candidato con la función hash. */
	int posicion_candidata = calcularPosicion(pid,pagina);

	int posOk = (posicion_candidata == 5);
	printf("Posicion candidata ok? %i\n", posOk);

	int posCandidataPerf = esPaginaCorrecta(posicion_candidata, 5, 5);
	printf("Es pagina correcta: %i\n", posCandidataPerf);

	int i;
	for(i = 0; i < CANTIDAD_DE_MARCOS; i++) {
		list_destroy(overflow[i]);
	}
	free(overflow);
}

void testLeerEscribirMemoriaConChars() {
	int tamanioCodigo = strlen("begin	variables a, f alocar a 100	abrir f LE /archivo.bin	wait mutexArch leer f a 10 prints n *a signal mutexArch wait b!pasadas=!pasadas + 1 prints n !pasadas signal b cerrar f	liberar a end");
	char* codigo = malloc(tamanioCodigo);
	codigo = "begin	variables a, f alocar a 100	abrir f LE /archivo.bin	wait mutexArch leer f a 10 prints n *a signal mutexArch wait b!pasadas=!pasadas + 1 prints n !pasadas signal b cerrar f	liberar a end";
	escribirTablaDePaginas(11,1,1);

	almacenarBytesEnPagina(1,1,0,tamanioCodigo,codigo);
	void* buffer = solicitarBytesDePagina(1,1,22,14);
}
/*
void testCache() {
	liberarProcesoDeCache(2);
	escribirCache(1,0,1);
	escribirCache(1,1,1);
	escribirCache(1,2,1);
	escribirCache(2,0,1);
	escribirCache(2,1,1);
	escribirCache(2,2,1);
	escribirCache(3,0,1);
	escribirCache(3,1,1);
	escribirCache(3,2,1);
	escribirCache(3,3,1);
	escribirCache(4,0,1);
	escribirCache(4,0,1);
	escribirCache(5,0,1);
	escribirCache(5,0,1);
	escribirCache(5,0,1);
	escribirCache(7,0,1);
	escribirCache(7,0,1);
	liberarProcesoDeCache(2);
	liberarProcesoDeCache(1);
	liberarProcesoDeCache(5);
	liberarProcesoDeCache(2);

	dumpCache();

	int* num = leerDeCache(3,2);
	log_warning(logger, "Contendio de lectura %i", num);
}

void testLeerEscribirMemoriaCacheConChars2() {
	escribirTablaDePaginas(8, 1, 0);
	escribirTablaDePaginas(9, 1, 1);
	escribirTablaDePaginas(10, 1, 2);
	escribirTablaDePaginas(11, 1, 3);

	char* codigo = malloc(320);
	codigo = "p11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111f";

	almacenarCodigo(1, 2, codigo);
	char* test = leerFrame(8,0,256);

	log_info(logger, "Codigo almacenado %s", test);
	char* test2 = leerFrame(9,0,256);
	log_info(logger, "codigo pag 2 %s",test2);
}
*/
