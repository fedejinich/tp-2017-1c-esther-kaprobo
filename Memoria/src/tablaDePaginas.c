/*
 * tablaDePaginas.c
 *
 *  Created on: 19/5/2017
 *      Author: utnso
 */


#include "tablaDePaginas.h"

void inicializarTablaDePaginas() {
	printf("Inicializando tabla de paginas...\n");
	int limiteMarco = getLimiteFrameByOffset(12); //Consigo el ultimo lugar en el cual voy a poder escribir segun un offset
	int frameAEscribir = 0; //Empiezo escribiendo el primer frame y luego incremento en base al espacio disponible
	int offset; //El desplazamiento dentro del frame a escribir

	int nroDeMarcoTabla = 0; //Es el nro de frame que va a aparecer en la tabla de paginas
	//Un while para recorrer todas las entradas de tabla de pagina que quiero llenr (500 entradas)
	while(nroDeMarcoTabla <= frames) {
		//Un for para recorrer cada frame que quiero llenar con entrada de tabla de pagina (un frame tiene 23 entradas aprox)
		for(offset = 0;offset < limiteMarco && nroDeMarcoTabla <= frames;offset = offset + sizeof(t_entradaTablaDePaginas)) {
			t_entradaTablaDePaginas* entrada = malloc(sizeof(t_entradaTablaDePaginas));

			int frame = nroDeMarcoTabla;
			int pid = -1;
			int pagina = 0;

			entrada->frame = frame;
			entrada->pid = pid;
			entrada->pagina = pagina;

			if(frame == 0 || frame == 100 || frame == 200 || frame == 300 ||frame == 400 ||frame == 500 || frame > 500)
				printf("Frame en tabla de paginas numero: %i\n",frame);

			escribir_frame(frameAEscribir,offset,sizeof(t_entradaTablaDePaginas),entrada);

			nroDeMarcoTabla = nroDeMarcoTabla + 1;

			free(entrada);
		}
		frameAEscribir = frameAEscribir + 1;
	}

	//Pongo el puntero para manejar la tabla de paginas en el inicio de la misma
	tablaDePaginas = (t_entradaTablaDePaginas*)&memoria[0];

	printf("Tabla de paginas inicializada.\n");
}
