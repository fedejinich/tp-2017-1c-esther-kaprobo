/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Consola.h"

//prueba de commit

int main(int argc, char **argv) {
	cargarConfiguracion(argv[0]);
	printf( "ip %i\n" ,ip_kernel);
	printf( "puerto %i\n", puerto_kernel);
}


void cargarConfiguracion(char * pathconf){
	char archi[14] = "consola.config";
	char * archivo = malloc(14 + strlen(pathconf)-7);
	printf ("%s\n", pathconf);
	memcpy(archivo, pathconf, strlen(pathconf)-7);
	printf ("%s\n", archivo);
	memcpy(archivo + strlen(pathconf)-7, archi, 14);
	printf ("%s\n", archivo);

	t_config * archivo_configuracion = config_create(archivo);
	ip_kernel = config_get_string_value(archivo_configuracion, "IP_KERNEL");
	printf("ip %c\n", ip_kernel);
	puerto_kernel = config_get_string_value(archivo_configuracion, "PUERTO_KERNEL");

	free(archivo);
}
