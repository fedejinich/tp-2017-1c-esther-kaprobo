/*
 * operacionesMemoriaCPU.c
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */


#include "operacionesMemoria-CPU.h"

void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	//PEDIDO DE LECTURA POR PARTE DE CPU
	//RETORNA NULL SI NO ENCUNETRA NADA
	retardo();
	void* buffer = -1;
	log_info(logger, "Solicitando bytes de PID: %i, pagina: %i, offset: %i y tamanio: %i", pid, pagina, offset, tamanio);

	if(estaEnCache(pid, pagina))
		leerDeCache(pid, pagina);
	else {
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginasHash(pid, pagina);
		if(entrada == EXIT_FAILURE) {
			log_error(logger, "Error en solicitar bytes de una pagina");
			return EXIT_FAILURE;
		}
		log_info(logger, "Los bytes del PID: %i, pagina: %i se encuentran en el frame %i", pid, pagina, entrada->frame);
		void* buffer = leerFrame(entrada->frame, offset, tamanio);
	}

	return buffer;
}

int almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
	retardo();
	log_info(logger, "Almacenando bytes de PID %i en pagina %i con offset %i y tamanio %i ...", pid, pagina, offset, tamanio);
	//escribirFrame();

	return EXIT_SUCCESS;
}
