
#include <stdio.h>
#include <stdlib.h>
#include "File_System.h"

int main(int argc, char **argv) {

	iniciarFileSystem();
	cargarConfiguracion(argv[0]);

	return EXIT_SUCCESS;
}


void iniciarFileSystem() {
	printf("%s", "\n\n====== INICIO FILE SYSTEM ======\n\n");
}

void cargarConfiguracion(char* pathconf) {

	printf("Cargando archivo de configuracion 'file_system.config'...\n\n");
	t_config* config = config_create(getenv("archivo_configuracion"));
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
