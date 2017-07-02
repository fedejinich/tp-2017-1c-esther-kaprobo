#include <stdint.h>
#include <commons/collections/list.h>
#include <parser/parser.h>


#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_


typedef struct __attribute__((packed))t_direccion{
	int pagina;
	int offset;
	int size;
} t_direccion;

typedef struct __attribute__((packed))t_contexto{
	int pos;
	t_list* args;
	t_list* vars;
	int retPos;
	t_direccion retVar;
	int sizeArgs;
	int sizeVars;
}t_contexto;

typedef struct __attribute__((packed)) t_entradaStack {
	t_list* args;
	t_list* vars;
	int retPos;
	t_direccion retVar;
} t_entradaStack;

typedef struct __attribute__((packed)) t_entradaVars {
	char id;
	t_direccion* direccion;
} t_entradaVars;

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

	int instrucciones;

	int exitCode;


	t_list* indiceStack;
}t_pcb;

typedef struct __attribute__((packed))t_proceso{
	t_pcb* pcb;
	int socketConsola;
	int socketCPU;
	bool abortado;
}t_proceso;


#endif
