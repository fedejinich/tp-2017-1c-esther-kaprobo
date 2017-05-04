#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
#include "src/Estructuras.h"
#include <pthread.h>
#include <commons/log.h>
#include <math.h>

#define MAX_CLIENTES 3
#define TAMANIODEPAGINA 256

//Logger
t_log* logger;



/*
 *
 * CONFIGURACION
 *
 * */

//VARIABLES
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
char* sem_ids[3];
int sem_inits[3];
char* shared_vars[2];
int stack_size;

//FUNCIONES
void cargarConfiguracion();
void mostrarConfiguracion();

/*
 *
 * SOCKETS
 *
 * */

//VARIABLES
un_socket fileSystem;
un_socket socketConsola;
un_socket socketCPU;
fd_set fds_activos; //Almacena los sockets a ser monitoreados por el select
un_socket socketCliente[MAX_CLIENTES];
un_socket socketMasGrande;
int numeroClientes = 0;
t_paquete* paqueteRecibido;
un_socket memoria;

//struct timeval timeout;

//FUNCIONES
void compactaClaves(int *tabla, int *n);
int dameSocketMasGrande (int *tabla, int n);
void prepararSocketsServidores();
void verSiHayNuevosClientes();
void nuevoCliente (int servidor, int *clientes, int *nClientes);
void procesarPaqueteRecibido(t_paquete* paqueteRecibido);
int pedirPaginasParaProceso();
un_socket conectarConLaMemoria();




