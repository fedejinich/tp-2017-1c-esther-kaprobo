#include <stdint.h>
#include <commons/collections/list.h>
#include <parser/parser.h>


#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

int tamanioPagina;

typedef struct __attribute__((packed)){
	int QUANTUM;
	int QUANTUM_SLEEP;
	int TAMANIO_PAG;
	int STACK_SIZE;
}t_datos_kernel;

typedef struct __attribute__((packed))t_direccion{
	int pagina;
	int offset;
	int size;
} t_direccion;


typedef struct __attribute__ ((packed))t_envioDeDatosKernelFSAbrir{
	int pid;
	char* path;
	char* permisos;
}t_envioDeDatosKernelFSAbrir;

typedef struct __attribute__ ((packed))t_envioDeDatosKernelFSLecturaYEscritura{
	int pid;
	int fd;
	int offset;
	int tamanio;
	char* contenido;
}t_envioDeDatosKernelFSLecturaYEscritura;

typedef struct __attribute__((packed))t_contexto{
	int pos;
	t_list* args;
	t_list* vars;
	int retPos;
	t_direccion* retVar;
	int sizeArgs;
	int sizeVars;
} t_contexto;

typedef struct __attribute__((packed))t_inicializar_proceso{
	int pid;
	int paginasTotales;
	int paginasStack;
	int paginasCodigo;
	int sizeCodigo;
	char* codigo;
} t_inicializar_proceso;


typedef struct __attribute__((packed)) t_variable {
	char etiqueta;
	t_direccion* direccion;
} t_variable;

typedef struct __attribute__((packed))t_pcb{
	int pid;
	int paginasDeMemoria;
	t_puntero_instruccion programCounter;
	int paginasDeCodigo;
	int *indiceDeCodigo;
	char* indiceEtiquetas;
	t_list* contextoActual;
	int sizeContextoActual;
	int sizeIndiceEtiquetas;
	int sizeIndiceDeCodigo;

	int exitCode;
	int sizeTotal;
}t_pcb;

typedef struct __attribute__((packed))t_proceso{
	t_pcb* pcb;
	int socketConsola;
	int socketCPU;
	bool abortado;
	int sizePaginasHeap;
}t_proceso;

void destruirPCB(t_pcb* pcb);
t_pcb* desserializarPCB(char* serializado);
char *serializarPCB(t_pcb *pcb);



#endif
