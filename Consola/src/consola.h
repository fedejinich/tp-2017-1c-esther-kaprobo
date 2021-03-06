#ifndef CONSOLA_H_
#define CONSOLA_H_

//#include <Commons_Kaprobo/Commons_Kaprobo.h>
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include "hiloPrograma.h"
#include "interfaceUsuario.h"
#include "auxiliaresConsola.h"
#include "signal.h"

#include "Commons_Kaprobo_Consola.h"

#define ARCHIVOLOG "Consola.log"
#define MAXPID 50

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






//Hilos
pthread_t threadNewProgram;



//Variables Consola
int ejecuta;
int opcion;



char * leerArchivo(FILE *archivo);


#endif
