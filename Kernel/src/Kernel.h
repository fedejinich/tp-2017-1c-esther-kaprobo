#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "src/Sockets_Kaprobo.h"

/*
 * VARIABLES
 * */

//Variables de configuracion
int puerto_prog;
int puerto_cpu;
int ip_memoria;
int puerto_memoria;
int ip_fs;
int puerto_fs;
int quantum;
int quantum_sleep;
char* algoritmo;
int grado_multiprog;

//Variables para Sockets
int servidor; //Identificador del socket del servidor
fd_set fds_activos; //Almacena los sockets a ser monitoreados por el select
struct timeval timeout;

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
