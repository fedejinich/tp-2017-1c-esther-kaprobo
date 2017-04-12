#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Sockets_Kaprobo.h"


char* ipFileSystem = "127.0.0.2";
int puerto;
char* puntoMontaje;
un_socket fileSystemServer; //socket que recibe conexiones (servidor)

void cargarConfiguracion(char* pathconf);


#endif
