#ifndef CPU_H_
#define CPU_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include "src/Estructuras.h"
#include <pthread.h>
#include <parser/parser.h>


char* ip_kernel;
int puerto_kernel;
char* ip_memoria;
int puerto_memoria;
signed int kernel;
signed int memoria;




t_pcb* pcb;

void iniciarCPU();
void cargarConfiguracion(char* pathconf);
int conectarConElKernel();
int conectarConMemoria();
t_pcb* deserializarPCB(char* buffer);

#endif
