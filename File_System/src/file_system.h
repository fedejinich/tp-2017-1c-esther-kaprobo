#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Sockets_Kaprobo.h"

/**
 * Variables
 */

	//Configuracion

	char* ipFileSystem = "127.0.0.2";
	int puerto;
	char* puntoMontaje;

	//Sockets

	un_socket fileSystemServer; //identificador del socket del file_system que recibe conexiones
	fd_set fds_activos; //Almacena los sockets a ser monitoreados por el select
	struct timeval timeout;


/**
 * Funciones
 */

	//Configuracion

	void cargarConfiguracion(char* pathconf);

	//Sockets

	un_socket iniciarFileSystemServer(char* ip, char* port);
	void prepararFileSystemServerParaEscuchar();
	void atenderYCrearConexiones();

#endif
