/*
 * hiloConsola.c
 *
 *  Created on: 4/5/2017
 *      Author: utnso
 */

#include "hiloConsola.h"

void* hiloConsolaMemoria() {
	inicializarVariables();
	comando = malloc(50);
	size_t size = 50;
	log_info(logger,"Inicio del hilo Consola");
	for(;;) {
		getline(&comando, &size, stdin);
		//opciones para consola memoria
		/*if(esDump(comando))
			dump();
		else if(string_equals_ignore_case(comando,"flush"))
			flush();
		else if(string_equals_ignore_case(comando,"size memory"))
			sizeMemory();
		else if(string_equals_ignore_case(comando,"retardo"))*/
		if(esRetardo(comando))
			retardo(comando);
		else if(esRetardoSolo(comando))
			log_info(logger,"Retardo de memoria: %i milisegundos.", retardo_memoria);
	}

}

void inicializarVariables() {
	retardoCommand = malloc(sizeof(string_length("retardo ")));
	retardoCommand = "retardo ";
}

bool esRetardo(char* comando) {
	char* posibleRetardo = string_substring(comando, 0, string_length(retardoCommand));
	return string_equals_ignore_case(posibleRetardo, retardoCommand);
}

bool esRetardoSolo(char* comando) {
	char* posibleRetardo = string_substring(comando, 0, (string_length(retardoCommand) - 1));
	return string_equals_ignore_case(posibleRetardo, string_substring(retardoCommand, 0, (string_length(retardoCommand) - 1)));
}

bool esDump(char* comando) {
	//implementar piola

	return string_equals_ignore_case(comando,"dump");
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

void retardo(char* comando) {
	char* nuevoRetardoString = string_substring(comando, string_length(retardoCommand), string_length(comando));
	if(isNumber(nuevoRetardoString)) {
		log_warning(logger, "Cambiando retardo de memoria...");
		log_warning(logger, "Retardo de memoria viejo: %i milisegundos.", retardo_memoria);
		int nuevoRetardo = atoi(nuevoRetardoString);
		retardo_memoria = nuevoRetardo;
		log_warning(logger, "Nuevo retardo de memoria: %i milisegundos.", retardo_memoria);
	} else {
		log_error(logger, "Escriba el comando nuevamente.");
	}
}

bool isNumber(char* palabra) {

	int tamanio = string_length(palabra)-1;

	t_list * lista_chars = list_create();

	int i;
	for (i = 0; i < tamanio; i++) {

		list_add(lista_chars, (void *) palabra[i]);

	}

	bool es_digito(void * elemento) {

		char * letra = (char *) elemento;

		return isdigit(letra);
	}

	bool resultado = list_all_satisfy(lista_chars, es_digito);

	free(lista_chars);

	return resultado;

}


