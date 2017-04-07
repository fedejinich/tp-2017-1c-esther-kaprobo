
#include <stdio.h>
#include <stdlib.h>
#include "File_System.h"

int main(int argc, char **argv) {

	cargarConfiguracion();

	return EXIT_SUCCESS;
}


void cargarConfiguracion() {

	printf("Cargando archivo de configuracion 'file_system.config'...\n\n");
	t_config* config = config_create("file_system.config");
	puerto = config_get_int_value(config, "PUERTO");
	puntoMontaje = config_get_string_value(config, "PUNTO_MONTAJE");

	if(config_has_property(config,"PUERTO"))
		printf("Puerto: %i \n",puerto);
	else
		printf("No esta setteado el PUERTO");
	if(config_has_property(config,"PUNTO_MONTAJE"))
		printf("Punto Montaje: %s \n",puntoMontaje);
	else
		printf("No esta setteado el PUNTO_MONTAJE");

}
