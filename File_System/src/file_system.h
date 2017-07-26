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
#include <semaphore.h>

#include <fcntl.h>
#include <sys/mman.h>


#define ARCHIVOLOG "File_System.log"


typedef uint32_t t_num;

/**
 * Variables
 */

	unsigned char* mmapDeBitmap;


	//Log
	t_log* logger;

	//Configuracion

	char* ipFileSystem = "127.0.0.1";


	t_bitarray* bitArray;

	//Semaforo
	pthread_mutex_t solicitud_mutex;

	//Sockets
	t_paquete* paquete;

	un_socket fileSystemServer; //identificador del socket del file_system que recibe conexiones
	un_socket socketKernel;



/**
 * Funciones
 */

	//Configuracion

	void cargarConfiguracion();
	void iniciarMetadataMap();

	//Log
	void crearArchivoLog();

	//Sockets

	void crearServidor();
	void atenderPedidos();


	//Archivos
	int existeArchivo(char* path);
	void crearArchivo(void* path);
	void borrarArchivo(void* path);

	void leerBitMap();
	char* leerBloquesArchivo(void* path, int offset, int size);
	void escribirBloquesArchivo(void* path, int offset, int size, char* buffer);
	char* leerArchivo(void* path);

#endif
