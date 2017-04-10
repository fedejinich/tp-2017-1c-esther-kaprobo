#ifndef CONSOLA_H_
#define CONSOLA_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Sockets_Kaprobo.h"


char* ip_kernel;
int puerto_kernel;
signed int kernel;


void iniciarConsola();
void cargarConfiguracion(char* pathconf);
int conectarConElKernel();

#endif
