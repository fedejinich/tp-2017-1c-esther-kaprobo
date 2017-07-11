#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "src/Commons_Kaprobo.h"
#include <pthread.h>
#include <time.h>
#include <ctype.h>
#include "hiloPrograma.h"
#include "interfaceUsuario.h"
#include "auxiliaresConsola.h"


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






//Hilos
pthread_t threadNewProgram;



//Variables Consola
int ejecuta;
int opcion;



#endif
