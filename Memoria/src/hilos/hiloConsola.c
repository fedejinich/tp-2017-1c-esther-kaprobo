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
		scanf("%s",&comando);
		//opciones para consola memoria
		if(string_equals_ignore_case(comando,"dump"))
			dump();
		else if(string_equals_ignore_case(comando,"flush"))
			flush();
		else if(string_equals_ignore_case(comando,"size"))
			size();
		else if(string_equals_ignore_case(comando,"retardo"))
			retardo();
	}

}

void dump() {

	printf("Implementa dump, pajero");

}

void flush() {

	printf("Implementa flush, pajero");

}

void size() {

	printf("Implementa size, pajero");

}

void retardo() {

	printf("Implementa retardo, pajero");

}


