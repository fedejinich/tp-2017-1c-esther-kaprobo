#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"
//#include "src/Estructuras.h"
#include <pthread.h>
#include <commons/log.h>
#include <math.h>
#include <semaphore.h>
#include <linux/inotify.h>
#include <event.h>
#include <commons/collections/node.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include <signal.h>


//DEFINE
#define MAX_CLIENTES 3
#define TAMANIODEPAGINA 256 //ESTE DATO LO TRAE DE MEMORIA
#define CONFIG_KERNEL_SOLO "kernel.config"
#define CONFIG_KERNEL "../configuracion/kernel.config"
#define CONFIG_PATH "../configuracion/"


/*
 *
 * ESTRUCTURAS
 *
 * */
typedef struct __attribute__((packed))t_pcb{
	int pid;
	int programCounter;
	//Falta referencia a tabla
	int stackPosition;
	int exitCode;
}t_pcb;

typedef struct __attribute__((packed))t_proceso{
	t_pcb* pcb;
	int socketConsola;
	int socketCPU;
	bool abortado;
}t_proceso;


//VARIABLES

//Globales

//PID dando vueltas
int cantidadDeProgramas  = 0;

//Numero pid a asignar a cada programa, no confundir
int pidcounter = 0;

//Logger
t_log* logger;

/*
 *
 * CONFIGURACION
 *
 * */

int puerto_prog;
int puerto_cpu;
char* ip_memoria;
int puerto_memoria;
char* ip_fs;
int puerto_fs;
int quantum;
int quantum_sleep;
int algoritmo;
int grado_multiprog;
char** sem_ids;
char ** sem_inits;
char** shared_vars;
int stack_size;

//Propias Configuracion KERNEL
int TAMPAG;
int* valor_semaforos;
int * valor_shared_vars;
bool hayConfiguracion = false;

//HILO NOTIFY

pthread_t hiloNotify;
pthread_t hiloEjecuta;
pthread_t hiloConsolaKernel;

int opcion;

//SEMAFOROS
sem_t sem_new;
sem_t sem_ready;
sem_t sem_cpu;
sem_t sem_exit;

pthread_mutex_t mutex_config;

pthread_mutex_t mutex_new, mutex_ready, mutex_exec, mutex_block, mutex_exit;

pthread_mutex_t mutexEjecuta;


//COLAS

t_queue * cola_new;
t_queue * cola_ready;
t_queue * cola_exec;
t_queue * cola_block;
t_queue * cola_exit;

int cant_new, cant_ready, cant_exec, cant_block, cant_exit = 0;


t_queue** colas_ios;


t_queue * cola_CPU_libres;

t_queue ** cola_semaforos;


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
un_socket memoria;

 //DESPUES HAY QUE HACER UN FIX DE ESTO Y DEFINIR ESTE STRUCT SOLO EN ESTRUCTURAS.H
//PERO AHORA EL PUTO DE C NO C PORQUE NO ME ETA DEJANDO

//FUNCIONES

//Varias Kernel
void inicializar();
void cargarConfiguracion();
void mostrarConfiguracion();
void verNotify();
void borrarArchivos();
int* convertirConfigEnInt(char** valores_iniciales);
int* iniciarSharedVars(char** variables_compartidas);
void * nalloc(int tamanio);

//Consola
void hiloConKer();
void mostrarMenu();
void mostrarListadoDeProcesos();
void mostrarUnaListaDeProcesos(t_queue* colaAMostrar);

//Sockets
void compactaClaves(int *tabla, int *n);
int dameSocketMasGrande (int *tabla, int n);
void prepararSocketsServidores();
void verSiHayNuevosClientes();
int nuevoClienteConsola (int servidor, int *clientes, int *nClientes);
void procesarPaqueteRecibido(t_paquete* paqueteRecibido, un_socket socketActivo);
un_socket conectarConLaMemoria();


//Creacion Programa
void hiloEjecutador();
void mandarAEjecutar(t_proceso* proceso, int socket);
int enviarCodigoAMemoria(char* codigo, int size, t_proceso* proceso, codigosMemoriaKernel codigoOperacion);
int pedirPaginasParaProceso(int pid);
t_proceso* crearPrograma(int socket);
int nuevoProgramaAnsisop(int* socket, t_paquete* paquete);

//Ejecucion programas

t_proceso* obtenerProcesoSocketCPU(t_queue *cola, int socketBuscado);
void pideSemaforo(int* socketActivo, t_paquete* paqueteRecibido);
void liberarSemaforo(int* socketActivo, t_paquete* paqueteRecibido);
t_pcb* desserializarPCB(char* serializado);
void destruirPCB(t_pcb* pcb);
int* buscarSemaforo(char*semaforo);
void escribeSemaforo(char* semaforo, int valor);
void imprimirConsola(int* socketActivo, t_paquete* paqueteRecibido);

void solicitaVariable(int* socketActivo, t_paquete* paqueteRecibido);
void escribirVariable(int* socketActivo, t_paquete* paqueteRecibido);
int* valorVariable(char* variable);



