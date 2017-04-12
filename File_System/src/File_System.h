#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>


int puerto;
char* puntoMontaje;

void iniciarFileSystem();
void cargarConfiguracion(char* pathconf);


#endif
