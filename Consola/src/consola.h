#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "src/Commons_Kaprobo.h"
#include <pthread.h>
#include <time.h>


#define ARCHIVOLOG "Consola.log"
#define MAXPID 300

//Configuracion
char* ip_kernel;
int puerto_kernel;

//log
t_log * log;

//Varias
char* script;
FILE* archivo;
char nomArchi[50];

//matriz

int matriz[MAXPID];

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

//Sockets
signed int kernel;
t_paquete *paqueteRecibido;

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
char * leerArchivo(FILE *archivo);
timeAct fechaYHora();
void mostrarEstadisticas(estadisticas estadisticasPrograma, int pid);


//Funciones Sockets
int conectarConElKernel();

#endif
