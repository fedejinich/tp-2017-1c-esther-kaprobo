
#include "file_system.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

	printf("%s", "\n\n====== INICIO FILE SYSTEM ======\n\n");
	cargarConfiguracion(argv[0]);
	iniciarServidor(ipFileSystem,puerto);

	return 0;
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

void iniciarServidor(char* ip, char* port) {

	printf("Iniciando file_system server\n");
	fileSystemServer = socket_escucha(ip, port);
	listen(fileSystemServer,100); //listen es el socket donde voy a escuchar y 100 es la cantidad maxima de conexiones en cola
	printf("Esperando conexiones\n");
	for(;;);

}
