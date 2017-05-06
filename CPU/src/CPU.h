#ifndef CPU_H_
#define CPU_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include "src/primitivas.h"
#include "src/Estructuras.h"
#include <pthread.h>
#include <parser/parser.h>

#define ARCHIVOLOG "Consola.log"

t_log* logger;
char* ip_kernel;
int puerto_kernel;
char* ip_memoria;
int puerto_memoria;
signed int kernel;
signed int memoria;

char* script;
FILE* archivo;
char nomArchi[50];




t_pcb* pcb;

void iniciarCPU();
void crearArchivoLog();
void prueboParser();
void cargarConfiguracion(char* pathconf);
int conectarConElKernel();
int conectarConMemoria();
t_pcb* deserializarPCB(char* buffer);
char * leerArchivo(FILE *archivo);

#endif
