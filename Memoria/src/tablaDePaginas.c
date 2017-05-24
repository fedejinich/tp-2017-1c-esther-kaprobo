/*
 * tablaDePaginas.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "tablaDePaginas.h"

void inicializarTablaDePaginas() {
	log_info(logger,"Inicializando tabla de paginas...");

	int limiteFrame = getLimiteFrameByOffset(12); //Consigo el ultimo lugar en el cual voy a poder escribir segun un offset
	int frameAEscribir = 0; //Empiezo escribiendo el primer frame y luego incremento en base al espacio disponible
	int offset; //El desplazamiento dentro del frame a escribir

	int nroDeFrameTablaDePaginas = 0; //Es el nro de frame que va a aparecer en la tabla de paginas

	//Un while para recorrer todas las entradas de tabla de pagina que quiero llenr (500 entradas)
	while(nroDeFrameTablaDePaginas <= frames) {
		//Un for para recorrer cada frame que quiero llenar con entrada de tabla de pagina (un frame tiene 20 entradas aprox)

		for(offset = 0; offset < limiteFrame && nroDeFrameTablaDePaginas <= frames; offset += sizeof(t_entradaTablaDePaginas)) {
			t_entradaTablaDePaginas* entradaTablaDePaginas = malloc(sizeof(t_entradaTablaDePaginas));

			int frame = nroDeFrameTablaDePaginas;
			int pid = -1;
			int pagina = 0;

			entradaTablaDePaginas->frame = frame;
			entradaTablaDePaginas->pid = pid;
			entradaTablaDePaginas->pagina = pagina;

			if(frame == 0 || frame == 100 || frame == 200 || frame == 300 ||frame == 400 ||frame == 500 || frame > 500)
				log_info(logger,"Entrada de tabla de paginas en memoria, entrada numero: %i",frame);

			escribir_frame(frameAEscribir,offset,sizeof(t_entradaTablaDePaginas),entradaTablaDePaginas);

			nroDeFrameTablaDePaginas = nroDeFrameTablaDePaginas + 1;

			free(entradaTablaDePaginas);
		}
		frameAEscribir++;
	}

	log_info(logger,"Tabla de paginas inicializada.");
}

t_entradaTablaDePaginas* getEntradaTablaDePaginas(int entrada) {
	int numeroDeEntradaEnFrame = numeroDeEntradaEnFrameBy(entrada);
	int numeroDeFrame = numeroDeFrameBy(entrada);

	entradaTablaPointer = &framePointer[numeroDeFrame];

	return &entradaTablaPointer[numeroDeEntradaEnFrame];
}

