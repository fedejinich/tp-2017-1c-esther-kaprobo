#ifndef CPU_H_
#define CPU_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>


char* ip_kernel;
int puerto_kernel;
int nucleos;


void iniciarCPU();
void cargarConfiguracion(char* pathconf);

#endif
