#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include "src/Sockets_Kaprobo.h"

/*
 * VARIABLES
 * */

//Variables de configuracion
int puerto;
int marcos;
int marco_size;
int entradas_cache;
int cache_x_proc;
char* reemplazo_cache;
int retardo_memoria;

//Logger

t_log* logger;

//Variables de Hilos

pthread_t servidorConexionesCPU;
pthread_t servidorConexionesKernel;

//Sockets
un_socket socketCPU;
un_socket socketKernel;

/*
 * FUNCIONES
 * */

//Funciones de configuracion
void cargarConfiguracion(char* pathconf);


//Funciones de hilos

void* hiloServidorCPU(void* arg);
void* hiloConexionCPU(void* socket);
void* hiloServidorKernel(void* arg);
void* hiloConexionKernel(void* socket);

