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



typedef struct{
	char* PUERTO_KERNEL;
	char* PUNTO_MONTAJE;
	int TAMANIO_BLOQUES;
	int CANTIDAD_BLOQUES;

}t_config_FS;



/**
 * Variables
 */

	char* pathBloques, *pathArchivos, *pathMetadata, *pathMetadataArchivo, *pathMetadataBitarray;


	//Log
	t_log* logger;

	//Configuracion


	t_config_FS* config;
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

	t_config_FS* cargarConfiguracion();
	void iniciarMetadata();
	void mkdirRecursivo(char* path);

	//Log
	void crearArchivoLog();

	//Sockets

	void crearServidor();
	void atenderPedidos();


	//Archivos
	bool existeArchivo(char* path);
	void crearArchivo(void* path);
	void borrarArchivo(void* path);

	void leerBitMap();
	char* leerBloquesArchivo(void* path, int offset, int size);
	void escribirBloquesArchivo(void* path, int offset, int size, char* buffer);
	char* leerArchivo(void* path);

#endif
