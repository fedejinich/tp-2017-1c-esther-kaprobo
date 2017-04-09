#ifndef CONSOLA_H_
#define CONSOLA_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>


char* ip_kernel;
int puerto_kernel;


void iniciarConsola();
void cargarConfiguracion(char* pathconf);

#endif
