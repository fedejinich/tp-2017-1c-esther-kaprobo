#include <stdint.h>
#include <commons/collections/list.h>

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_


typedef struct __attribute__((packed))t_pcb{
	int pid;
	int pageCounter;
	//Falta referencia a tabla
	int stackPosition;
	int exitCode;
}t_pcb;

enum {
	PEDIDO_DE_PAGINAS,
	PEDIDO_DE_PAGINAS_OK,
	PEDIDO_DE_PAGINAS_FALLO
} mensajesKernelMemoria;

#endif
