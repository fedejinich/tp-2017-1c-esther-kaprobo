/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

int puerto_kernel;
char* ip_kernel;

int main(int argc, char **argv) {

	iniciarConsola();
	cargarConfiguracion();
}
void iniciarConsola(){
	printf("%s", "\n\n====== INICIO ======\n\n");
}
void cargarConfiguracion(){
	t_config* config = config_create("consola.config");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	printf("Ip Kernel: %s \n",ip_kernel);
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	printf("Puerto Kernel: %i \n",puerto_kernel);
}
