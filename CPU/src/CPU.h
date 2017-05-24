#ifndef CPU_H_
#define CPU_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include "primitivas.h"
#include "src/Estructuras.h"
#include <pthread.h>
#include <parser/parser.h>
#include <signal.h>

#define ARCHIVOLOG "CPU.log"

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


t_paquete* paquete_recibido;
int sigusr1_desactivado;

t_pcb* pcb;

void iniciarCPU();
void crearArchivoLog();
void prueboParser();
void cargarConfiguracion();
void ejecutarArchivo(FILE *archivo);
int conectarConElKernel();
int conectarConMemoria();
t_pcb* deserializarPCB(char* buffer);
char * leerArchivo(FILE *archivo);
char* depurarSentencia(char* sentencia);

void sig_handler(int signo);
void sig_handler2(int signo);



#endif
