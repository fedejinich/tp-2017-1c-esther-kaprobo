/*
 * operacionesMemoriaCPU.c
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */


#include "operacionesMemoria-CPU.h"



int almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
	retardo();
	log_info(logger, "Almacenando %i bytes de PID %i en pagina %i con offset %i ...", tamanio, pid, pagina, offset);

	int frame = getFrameByPIDPagina(pid, pagina);

	if(!superaLimiteFrame(offset, tamanio) && frame != EXIT_FAILURE_CUSTOM) {
		escribirFrame(frame, offset, tamanio, buffer);
		log_debug(logger, "Almacenados %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);

		return EXIT_SUCCESS_CUSTOM;
	}

	log_error(logger, "No se pudieron almacenar %i bytes de PID %i en pagina %i con offset %i", tamanio, pid, pagina, offset);
	//escribirFrame();


}
