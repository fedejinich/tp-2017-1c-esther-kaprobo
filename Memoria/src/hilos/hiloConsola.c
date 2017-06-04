/*
 * hiloConsola.c
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#include "hiloConsola.h"

void* hiloConsolaMemoria() {
	log_info(logger,"Inicio del hilo Consola");
	while(1) {
		int tiempo = 0;
		scanf("%s",&comando);
		//opciones para consola memoria
		if(string_equals_ignore_case(comando,"dump"))
			dump();
		else if(string_equals_ignore_case(comando,"flush"))
			flush();
		else if(string_equals_ignore_case(comando,"size memory"))
			sizeMemory();
		else if(string_equals_ignore_case(comando,"retardo"))
			retardo(tiempo);
	}

}

void dump() {

	printf("Implementa dump, pajero");

}

void flush() {

	printf("Implementa flush, pajero");

}

void sizeMemory() {

	printf("Implementa size, pajero");

}

void retardo(int retardo) {

	printf("Implementa retardo, pajero");

}


