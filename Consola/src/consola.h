#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "src/Commons_Kaprobo.h"
#include <pthread.h>
#include <time.h>
#include <ctype.h>


#define ARCHIVOLOG "Consola.log"
#define MAXPID 300

//Configuracion
char* ip_kernel;
int puerto_kernel;

//log
t_log * logger;

//Varias
char* script;
FILE* archivo;
char nomArchi[50];


//Semaforo
pthread_mutex_t mutexConexion;
pthread_mutex_t mutexEjecuta;
//matriz

int matriz[MAXPID];
pthread_t matrizHilos[MAXPID];

typedef struct{
	int d;
	int m;
	int y;
	int H;
	int M;
	int S;
}timeAct;

//estadisticas
typedef struct{
	timeAct fechaYHoraInicio;
	timeAct fechaYHoraFin;
	int impresiones;
	int tiempo;
}estadisticas;



//Hilos
pthread_t threadNewProgram;



//Variables Consola
int ejecuta;
int opcion;

//Funciones Consola
void iniciarConsola();
void limpiarArchivos();
void crearArchivoLog();
void cargarConfiguracion();
void iniciarPrograma();
void finalizarPrograma();
void desconectarConsola();
void limpiarMensajes();
void hiloNuevoPrograma();
void mostrarMenu();
char * leerArchivo(FILE *archivo);
timeAct fechaYHora();
void mostrarEstadisticas(estadisticas estadisticasPrograma, int pid);


//Funciones Sockets
int conectarConElKernel();

#endif
