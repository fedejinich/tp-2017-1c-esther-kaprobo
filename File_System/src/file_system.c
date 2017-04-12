
#include <stdio.h>
#include <stdlib.h>
#include "File_System.h"

int main(int argc, char **argv) {

	iniciarFileSystem();
	cargarConfiguracion(argv[0]);
	//kernel = conectarseConElKernel();
	return 0;
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

//funcion que conecta Consola con Kernel utilizando sockets
int conectarConElKernel(){
	/*printf("Inicio de conexion con Kernel\n");
	// funcion deSockets
	//kernel = conectar_a(ip_kernel,puerto_kernel);

	if (kernel==0){
		printf("CONSOLA: No se pudo conectar con el Kernel\n");
		exit (EXIT_FAILURE);
	}

	printf("CONSOLA: Conectado con kernel\n");
	return kernel;*/

	return 0;
}
