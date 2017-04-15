#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Sockets_Kaprobo.h"
#include <pthread.h>
#include <commons/log.h>


/**
 * Variables
 */

	//Log
	t_log* logger;

	//Configuracion

	char* ipFileSystem = "127.0.0.2";
	int puerto;
	char* puntoMontaje;

	//Sockets

	un_socket fileSystemServer; //identificador del socket del file_system que recibe conexiones
	un_socket socketKernel;

	//Hilos

	pthread_t servidorConexionesKernel;

/**
 * Funciones
 */

	//Configuracion

	void cargarConfiguracion(char* pathconf);

	//Sockets

	un_socket iniciarFileSystemServer(char* ip, char* port);
	void prepararFileSystemServerParaEscuchar();
	void atenderYCrearConexiones();

	//Hilos
	void* hiloServidorKernel(void* arg);
	void* hiloConexionKernel(void* socket);


#endif
