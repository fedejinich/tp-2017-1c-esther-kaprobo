#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include <pthread.h>
#include <commons/log.h>

/*
 * VARIABLES
 * */

//Logger
t_log* logger;

//Variables de configuracion
int puerto_prog;
int puerto_cpu;
char* ip_memoria;
int puerto_memoria;
char* ip_fs;
int puerto_fs;
int quantum;
int quantum_sleep;
char* algoritmo;
int grado_multiprog;

// Hilos
pthread_t servidorConexionesConsola, servidorConexionesCPU, clienteConexionMemoria, clienteConexionFileSystem;


//Variables para Sockets
un_socket fileSystem;
int servidor; //Identificador del socket del servidor
fd_set fds_activos; //Almacena los sockets a ser monitoreados por el select
struct timeval timeout;


int socketConsola, socketCPU;

/*
 * FUNCIONES
 * */

//Funciones de configuracion
void cargarConfiguracion();
void mostrarConfiguracion();

//Funciones de sockets
int inicializarServidor();
void prepararservidoretServidorParaEscuchar();
void atenderYCrearConexiones();
char* recibirMensajeCliente();

//Funciones de Hilos
void* hiloServidorConsola(void *arg);
void* hiloConexionConsola(void *socket);
void* hiloServidorCPU(void *arg);
void* hiloConexionCPU(void *socket);
void* hiloClienteFileSystem(void* arg);
void* hiloConexionFileSystem(void *arg);
void* hiloConexionMemoria(void *arg);

