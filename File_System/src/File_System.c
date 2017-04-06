/*
 ============================================================================
 Name        : File_System.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "File_System.h"

int main(int argc, char **argv) {

	cargarConfiguracion();

	return EXIT_SUCCESS;
}


void cargarConfiguracion() {

	t_config* config = config_create("file_system.config");
	puerto = config_get_string_value(config, "PUERTO");
	printf("Puerto: %s \n",puerto);
	puntoMontaje = config_get_string_value(config, "PUNTO_MONTAJE");
	printf("Punto Montaje: %s \n",puntoMontaje);

}
