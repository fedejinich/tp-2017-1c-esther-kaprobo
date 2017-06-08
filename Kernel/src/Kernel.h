#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
//#include "src/Estructuras.h"
#include <pthread.h>
#include <commons/log.h>
#include <math.h>

#define MAX_CLIENTES 3
#define TAMANIODEPAGINA 256

int cantidadDeProgramas;

typedef struct __attribute__((packed))t_pcb{
	int pid;
	int pageCounter;
	//Falta referencia a tabla
	int stackPosition;
	int exitCode;
}t_pcb;

typedef struct __attribute__((packed))t_proceso{
	t_pcb* pcb;
	un_socket socketConsola;
	un_socket socketCPU;
}t_proceso;

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
void inicializar();
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

typedef struct {
	int pid;
	int paginasAPedir;
} t_pedidoDePaginasKernel; //DESPUES HAY QUE HACER UN FIX DE ESTO Y DEFINIR ESTE STRUCT SOLO EN ESTRUCTURAS.H
//PERO AHORA EL PUTO DE C NO C PORQUE NO ME ESTA DEJANDO

//struct timeval timeout;

//FUNCIONES
void compactaClaves(int *tabla, int *n);
int dameSocketMasGrande (int *tabla, int n);
void prepararSocketsServidores();
void verSiHayNuevosClientes();
void nuevoCliente (int servidor, int *clientes, int *nClientes);
void procesarPaqueteRecibido(t_paquete* paqueteRecibido);
int pedirPaginasParaProceso(int pid);
un_socket conectarConLaMemoria();

/*
 *
 * COLAS
 *
 */
t_list* colaNew;

t_list* colaReady;

t_list* colaExec;

t_list* colaExit;

/*
 * ERRORES
 */
void informarAConsolaDeError(int resultadoPedidoPaginas);
