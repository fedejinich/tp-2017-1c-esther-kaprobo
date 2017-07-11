#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "src/Commons_Kaprobo.h"


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



typedef struct __attribute__((packed))t_entradaTablaGlobalArchivos{
	char* path;
	int open;
}t_entradaTablaGlobalArchivos;

typedef struct __attribute__((packed))t_tablaDeArchivosDeUnProceso{
	char* flags;
	int globalFD;
}t_tablaDeArchivosDeUnProceso;

typedef struct __attribute__((packed))t_entradaTablaDeArchivosPorProceso{
	int pid;
	t_list* tablaDeUnProceso;
}t_entradaTablaDeArchivosPorProceso;


//VARIABLES

//Globales

volatile int flag = 0;
int flagCPU=0;
//PID dando vueltas
int cantidadDeProgramas  = 0;

//Numero pid a asignar a cada programa, no confundir
int pidcounter = 1;

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
pthread_t hiloPCP;
pthread_t hiloConsolaKernel;

int opcion, opcionPID, opcionMultiProg;

//SEMAFOROS
sem_t sem_new;
sem_t sem_ready;
sem_t sem_cpu;
sem_t sem_exit;

bool estadoPlanificacion = true;
bool yaMeFijeReady = false;

pthread_mutex_t mutex_config;

pthread_mutex_t mutex_new, mutex_ready, mutex_exec, mutex_block, mutex_exit;

pthread_mutex_t mutexEjecuta;

pthread_mutex_t mutexGradoMultiprogramacion;

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

//CAPA FILESYSTEM
t_list* tablaGlobalDeArchivos;
t_list* tablaDeArchivosPorProceso;

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


//HEAP
typedef struct{
	int pagina;
	int pid;
	int disponible;
}t_adminHeap;



typedef struct __attribute__ ((packed))t_datosHeap{
	int pagina;
	int offset;
}t_datosHeap;

t_list* listaAdminHeap;

pthread_mutex_t mutex_listaHeap;




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
void mostrarInformacionDeProceso(int pid);
void cambiarGradoMultiprogramacion(int gradoNuevo);
void forzarFinalizacionDeProceso(int pid);
t_queue* buscarProcesoEnLasColas(int pid);
t_proceso* obtenerProcesoPorPID(t_queue *cola, int pid);

//Sockets
void compactaClaves(int *tabla, int *n);
int dameSocketMasGrande (int *tabla, int n);
void prepararSocketsServidores();
void verSiHayNuevosClientes();
int nuevoClienteConsola (int servidor, int *clientes, int *nClientes);
void procesarPaqueteRecibido(t_paquete* paqueteRecibido, un_socket socketActivo);
un_socket conectarConLaMemoria();


//Creacion Programa
void planificadorCortoPlazo();
void mandarAEjecutar(t_proceso* proceso, int socket);

int inicializarProcesoYAlmacenarEnMemoria(char* codigo, int size, t_proceso* proceso);
int almacenarCodigoMemoria(int pid, int paginasCodigo, char* codigo);
t_list* getCodigosParciales(char* codigo, int size);

int pedirPaginasParaProceso(int pid);
t_proceso* crearPrograma(int socketC , t_paquete* paquete);
int nuevoProgramaAnsisop(un_socket socket, t_paquete* paquete);
int ** desseralizarInstrucciones(t_size instrucciones, t_intructions* instrucciones_serializados);

//Ejecucion programas

t_proceso* obtenerProcesoSocketCPU(t_queue *cola, int socketBuscado);
void pideSemaforo(int* socketActivo, t_paquete* paqueteRecibido);
void liberarSemaforo(int* socketActivo, t_paquete* paqueteRecibido);
t_pcb* desserializarPCB(char* serializado);
void destruirPCB(t_pcb* pcb);
int* buscarSemaforo(char*semaforo);
void escribeSemaforo(char* semaforo, int valor);
void imprimirConsola(int* socketActivo, t_escribirArchivo* imprimir);

void solicitaVariable(int* socketActivo, t_paquete* paqueteRecibido);
void escribirVariable(int* socketActivo, t_paquete* paqueteRecibido);
int* valorVariable(char* variable);

void bloqueoSemaforo(t_proceso* proceso, char* semaforo);

void finalizarProgramaKernel(un_socket socket, t_paquete* paquete, ExitCodes exitCode);
void abortar(t_proceso* proceso);

void solicitudDeEscrituraArchivo(un_socket socketActivo, t_paquete* paqueteRecibido);

void finalizarProceso(t_proceso* proceso, ExitCodes exitCode);


//HEAP
void reservarHeap(un_socket socketCPU, t_paquete * paqueteRecibido);
void procesoLiberaHeap(un_socket socketCPU, t_paquete * paqueteRecibido);
int reservarBloqueHeap(int pid, int size, t_datosHeap* puntero);
t_datosHeap* verificarEspacioLibreHeap( int pid, int tamanio);
int reservarPaginaHeap(int pid,int pagina);
void compactarPaginaHeap( int pagina, int pid);

int paginaHeapConBloqueSuficiente(int posicionPaginaHeap, int pagina, int pid, int tamanio);
codigosKernelCPU liberarBloqueHeap(int pid, int pagina, int offset);




//CAPA Filesystem
void abrirArchivo(un_socket socketActivo, t_paquete* paquete);
bool validarPermisoDeApertura(int pid, char* path, char* permisos);
bool existeArchivo(char* path);
int chequearTablaGlobal(char* path);
int buscarEntradaEnTablaGlobal(char* path);
t_tablaDeArchivosDeUnProceso* obtenerEntradaTablaArchivosDelProceso(int pid, int fd);
t_entradaTablaGlobalArchivos* obtenerEntradaTablaGlobalDeArchivos(t_tablaDeArchivosDeUnProceso* entradaTablaDelProceso);
void borrarArchivoDeTabla(int pid, int fd);

void escribirArchivo(int* socketActivo, t_paquete* paquete);
void leerArchivo(int* socketActivo, t_paquete* paquete);
void cerrarArchivo(int* socketActivo, t_paquete* paquete);
