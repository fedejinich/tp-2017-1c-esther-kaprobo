/*
 * operacionesMemoriaCPU.c
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */


#include "operacionesMemoria-CPU.h"

void solicitarBytesDePagina(int pid, int pagina, int offset, int tamanio) {
	//PEDIDO DE LECTURA POR PARTE DE CPU
	retardo();

	if(estaEnCache(pid, pagina))
		leerDeCache(pid, pagina);
	else {

	}
}

void almacenarBytesEnPagina(int pid, int pagina, int offset, int tamanio, void* buffer) {
	//PEDIDO DE ESCRITURA POR PARTE DE CPU
	retardo();

}
