/*
 * abstractOperaciones.c
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */

#include "abstractOperaciones.h"

void retardo() {
	log_info(logger, "sleep(%i)", retardo_memoria);
	sleep(retardo_memoria/1000);
}

void* solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	retardo(); //EL RETARDO VA ACA O VA CUANDO EMPIEZA EL ELSE? LA CACHE SE AFECTA POR EL RETARDO?
	void* buffer = -1;
	log_info(logger, "Solicitando bytes de PID: %i, pagina: %i, offset: %i y tamanio: %i", pid, pagina, offset, tamanio);

	if(estaEnCache(pid, pagina))
		leerDeCache(pid, pagina);
	else {
		retardo();
		t_entradaTablaDePaginas* entrada = getEntradaTablaDePaginasHash(pid, pagina);

		if(entrada == EXIT_FAILURE_CUSTOM) {
			log_error(logger, "Error en solicitar bytes de una pagina");
			return EXIT_FAILURE_CUSTOM;
		}

		log_info(logger, "Los bytes del PID: %i, pagina: %i se encuentran en el frame %i", pid, pagina, entrada->frame);
		void* buffer = leerFrame(entrada->frame, offset, tamanio);
		escribirCache(pid, pagina, buffer);
	}

	return buffer;
}
