#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include <pthread.h>
#include <commons/log.h>


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


#endif
