#ifndef CPU_H_
#define CPU_H_


#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include "primitivas.h"
#include "src/Estructuras.h"
#include "auxiliaresPrimitivas.h"
#include <pthread.h>
#include <parser/parser.h>
#include <signal.h>
#include <commons/log.h>
#include <math.h>

#define ARCHIVOLOG "CPU.log"

//log
t_log* logger;

//sockets
char* ip_kernel;
int puerto_kernel;
char* ip_memoria;
int puerto_memoria;
signed int kernel;
signed int memoria;
t_paquete* paquete_recibido;




//señales
int sigusr1_desactivado;

//semaforos
pthread_mutex_t mutex_pcb;

//Varios
char* script;
FILE* archivo;
char nomArchi[50];
t_pcb* pcb;
int algoritmo;
t_paquete* paq_algoritmo;

//Variables
int quantum;
int quantum_sleep;
int stack_size;
int tamanio_pag;
int programaBloqueado;
int programaAbortado;
int programaFinalizado;
int var_max;


void iniciarCPU();
void crearArchivoLog();
void prueboParser();
void cargarConfiguracion();
void ejecutarArchivo(FILE *archivo);
int conectarConElKernel();
int conectarConMemoria();
t_pcb* deserializarPCB(char* buffer);
//char * leerArchivo(FILE *archivo);
char* depurarSentencia(char* sentencia);



void sig_handler(int signo);
void sig_handler2(int signo);
void asignarDatosDelKernel(t_paquete* datos_kernel);


char* leer(int pid, int pagina, int offset, int tamanio);



#endif
