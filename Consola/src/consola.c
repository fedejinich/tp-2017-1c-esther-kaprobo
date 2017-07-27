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
	signal(SIGINT, desconexionConsola);
	limpiarArchivos();
	iniciarConsola();
	crearArchivologgerConsola();
	cargarConfiguracion();
	interface();


	return 0;
}


//Funcion leer archivo y armar script
char * leerArchivo(FILE *archivo){
	fseek(archivo, 0, SEEK_END);
	long fsize = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *script = malloc(fsize + 1);
	fread(script, fsize, 1, archivo);
	script[fsize] = '\0';
	return script;
}
