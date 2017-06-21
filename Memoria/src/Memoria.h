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
#include "operaciones/abstractOperaciones.h"
#include "funcionHash/funcionHash.h"
#include "cache/cache.h"
#include "operaciones/operacionesMemoria-CPU.h"
#include "operaciones/operacionesMemoria-Kernel.h"


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
un_socket socketClienteKernel;

/*
 * FUNCIONES
 * */

//Funciones de configuracion
void grandMalloc();
void cargarConfiguracion();


//Funciones
void iniciarSeniales();
void iniciarHilos();

void testFuncionHashObtengoPosicionCandidataOk();




