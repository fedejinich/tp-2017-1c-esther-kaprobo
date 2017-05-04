#ifndef SRC_TABLADEPAGINAS_ESTRUCTURAS_H_
#define SRC_TABLADEPAGINAS_ESTRUCTURAS_H_

#include <stdbool.h>

#include <commons/collections/list.h>
#include "Memoria.h"


typedef struct {
	int pid;
	int pagina;
	int marco;
	bool uso;
} t_entradaTablaDePaginas;



#endif /* SRC_TABLADEPAGINAS_ESTRUCTURAS_H_ */
