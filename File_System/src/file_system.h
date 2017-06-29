#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include <pthread.h>
#include <commons/log.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/process.h>

#define ARCHIVOLOG "File_System.log"
#define KernelValidacion 11

/**
 * Variables
 */

	//Log
	t_log* logger;

	//Configuracion

	char* ipFileSystem = "127.0.0.1";
	int puerto;
	char* puntoMontaje;
	int tamanioBloques;
	int cantidadBloques;
	t_bitarray* bitMap;

	//Sockets
	t_paquete* paquete;

	un_socket fileSystemServer; //identificador del socket del file_system que recibe conexiones
	un_socket socketKernel;

	//Hilos

	pthread_t servidorConexionesKernel;

/**
 * Funciones
 */

	//Configuracion

	void cargarConfiguracion();

	//Log
	void crearArchivoLog();

	//Sockets

	un_socket iniciarFileSystemServer(char* ip, char* port);
	void prepararFileSystemServerParaEscuchar();
	void atenderYCrearConexiones();

	//Hilos
	void* hiloServidorKernel(void* arg);
	void* hiloConexionKernel(void* socket);

	//Archivos
	void crearArchivo(void* path);
	void borrarArchivo(void* path);
	void leerMetadataArchivo();
	void leerBitMap();
	char* leerBloquesArchivo(void* path, int offset, int size);
	void escribirBloquesArchivo(void* path, int offset, int size, char* buffer);
	char* leerArchivo(void* path);

#endif
