/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "consola.h"


int main(int argc, char **argv) {
	limpiarArchivos();
	iniciarConsola();
	crearArchivologgerConsola();
	cargarConfiguracion();
	interface();


	return 0;
}


