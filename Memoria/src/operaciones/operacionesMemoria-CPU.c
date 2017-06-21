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
	void* buffer;
	log_info(logger, "Solicitando bytes de PID: %i, pagina: %i, offset: %i y tamanio: %i", pid, pagina, offset, tamanio);

	if(estaEnCache(pid, pagina))
		leerDeCache(pid, pagina);
	else {
		//completar
	}


}

bool almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
	retardo();

}
