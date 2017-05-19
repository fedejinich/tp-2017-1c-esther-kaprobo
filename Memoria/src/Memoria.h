#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <signal.h>
#include <commons/collections/list.h>


#include "src/Commons_Kaprobo.h"
#include "estructuras.h"
#include "hilos/hiloCPU.h"
#include "hilos/hiloKernel.h"
#include "hilos/hiloConsola.h"
#include "funcionesAuxiliares/funcionesAuxiliares.h"
#include "tablaDePaginas.h"


/*
 * VARIABLES
 * */

//Variables de configuracion
int puerto;
int frames;
int frame_size;
int entradas_cache;
int cache_x_proc;
char* reemplazo_cache;
int retardo_memoria;

//Memoria
void* memoria;
int ultimaPosicion;//puntero que apunta a la ultima posicion utilizada de memoria
int tamanioMemoria;

//Logger

t_log* logger;

//Variables de Hilos

pthread_t servidorConexionesCPU;
pthread_t servidorConexionesKernel;
pthread_t consolaMemoria;

//Sockets
un_socket socketCPU;
un_socket socketKernel;

/*
 * FUNCIONES
 * */

//Funciones de configuracion
void grandMalloc();
void cargarConfiguracion();
void inicializarTablaDePaginas();
void inicializarMemoria();

//Funciones
void iniciarSeniales();
void iniciarHilos();


void alojarEnMemoria(int pid, int paginasRequeridas);
bool espacioDisponible(int pid, int paginasRequeridas);
void agregarEntradaEnTablaDePaginas(int pid, int paginasRequeridas);
int getMarcoDisponible();

//Funciones de hilos

void* hiloServidorCPU(void* arg);
void* hiloConexionCPU(void* socket);
void* hiloServidorKernel(void* arg);
void* hiloConexionKernel(void* socket);
void* hiloConsolaMemoria();


