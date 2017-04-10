/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : CPU
 ============================================================================
 */

#include "CPU.h"

int main(int argc, char **argv) {
	iniciarCPU();
	cargarConfiguracion(argv[0]);
	//kernel = conectarConElKernel();
	return 0;
}

void iniciarCPU(){
	printf("%s", "\n\n====== INICIO CPU ======\n\n");

}
void cargarConfiguracion(char* pathconf){
	t_config* config = config_create(getenv("archivo_configuracion"));
	nucleos = config_get_int_value(config, "NUCLEOS");
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	printf("NUCLEOS: %i \n", nucleos);
	printf("IP KERNEL: %s \n", ip_kernel);
	printf("PUERTO_KERNEL: %i \n", puerto_kernel);
	printf("El archivo de configuracion fue cargado con exito\n");
}
