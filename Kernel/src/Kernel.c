/*
 ============================================================================
 Name        : Kernel.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"

int main(int argc, char **argv) {
	cargarConfiguracion();
	mostrarConfiguracion();
	return EXIT_SUCCESS;
}

void cargarConfiguracion() {

	printf("Cargando archivo de configuracion 'kernel.config'\n\n");
	t_config* config = config_create("kernel.config");
	puerto_prog = config_get_int_value(config, "PUERTO_PROG");
	puerto_cpu = config_get_int_value(config, "PUERTO_CPU");
	ip_memoria = config_get_int_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
	ip_fs = config_get_int_value(config, "IP_FS");
	puerto_fs = config_get_int_value(config, "PUERTO_FS");
	quantum = config_get_int_value(config, "QUANTUM");
	quantum_sleep = config_get_int_value(config, "QUANTUM_SLEEP");
	algoritmo = config_get_string_value(config, "ALGORITMO");
	grado_multiprog = config_get_int_value(config, "GRADO_MULTIPROG");
}

void mostrarConfiguracion(){
	printf("Puerto Prog: %i \n",puerto_prog);
	printf("Puerto CPU: %i \n",puerto_cpu);
	printf("IP Memoria: %i \n",ip_memoria);
	printf("Puerto Memoria: %i \n",puerto_memoria);
	printf("IP File System: %i \n",ip_fs);
	printf("Puerto File System: %i \n",puerto_fs);
	printf("Quantum: %i \n",quantum);
	printf("Quantum Sleep: %i \n",quantum_sleep);
	printf("Algoritmo: %s \n",algoritmo);
	printf("Grado Multiprogramacion: %i \n",grado_multiprog);
}
