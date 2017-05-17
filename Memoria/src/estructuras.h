#ifndef SRC_TABLADEPAGINAS_ESTRUCTURAS_H_
#define SRC_TABLADEPAGINAS_ESTRUCTURAS_H_

#include <stdbool.h>

#include <commons/collections/list.h>
#include "Memoria.h"


typedef struct {
	int marco;
	int pid;
	int pagina;
} t_entradaTablaDePaginas;



//4 bytes pid, 4 bytes pagina y 4 bytes marco = 12 bytes entrada
//12 bytes * 500 = 6000 bytes es lo que ocupa la tabla de paginas dentro del grandMalloc (gran bloque de memoria continua)





#endif /* SRC_TABLADEPAGINAS_ESTRUCTURAS_H_ */
