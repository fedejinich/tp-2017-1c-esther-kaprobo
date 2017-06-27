#ifndef SRC_TABLADEPAGINAS_ESTRUCTURAS_H_
#define SRC_TABLADEPAGINAS_ESTRUCTURAS_H_
#include <stdbool.h>
#include <commons/collections/list.h>
#include "Memoria.h"


/*
 * VARIABLES
 * */

//Variables de configuracion
int puerto;
int frames;
int frame_size;
int entradas_cache;
int cache_x_proc;
char* reemplazo_cache;
int retardo_memoria;

//Memoria
int tamanioMemoria;

//Logger

t_log* logger;

//Variables de Hilos

pthread_t servidorConexionesCPU;
pthread_t servidorConexionesKernel;
pthread_t consolaMemoria;

//Sockets
un_socket socketCPU;
un_socket socketKernel;
un_socket socketClienteKernel;

typedef struct {
	char data[256];
} t_frame;

int framesLibres;
int framesOcupados;

t_frame* memoria;
t_frame* framePointer;




//4 bytes pid, 4 bytes pagina y 4 bytes marco = 12 bytes entrada
//12 bytes * 500 = 6000 bytes es lo que ocupa la tabla de paginas dentro del grandMalloc (gran bloque de memoria continua)
#endif /* SRC_TABLADEPAGINAS_ESTRUCTURAS_H_ */
