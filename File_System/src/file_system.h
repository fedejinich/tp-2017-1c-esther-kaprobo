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

	//Pedidos

	void validarArchivo(t_paquete* paquete);
	void crearArchivo(t_paquete* paquete);
	void borrarArchivo(t_paquete* paquete);
	void obtenerDatos(t_paquete* paquete);
	void guardarDatos(t_paquete* paquete);









	//Archivos
	bool existeArchivo(char* path);

	int buscarBloqueLibre();
	void escribirValorBitarray(bool valor, int pos);
	char* generarPathArchivo(char* path);


	int string_pos_char(char* string, char caracter);
	void leerArchivo(int bloque, char* buffer, int size, int offset);
	char* generarPathBloque(int num_bloque);
	int cantidadBloques(char** bloques);
	void aumentarTamanioArchivo(int offset, int size, char* path);
	void escribirEnArchivo(int bloque, char* buffer, int size, int offset);
	int reservarNuevoBloque(char* pathArchivo);

	//AUX

	void config_set_value(t_config* self, char* key, char*value);
	int config_save(t_config* self);
	int config_save_in_file(t_config* self, char* path);

#endif
