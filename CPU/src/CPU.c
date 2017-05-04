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
	t_paquete* paquete_recibido;
	iniciarCPU();
	cargarConfiguracion(argv[0]);
	kernel = conectarConElKernel();
	memoria = conectarConMemoria();
	while (1){
		paquete_recibido = recibir(kernel);
		pcb = deserializarPCB(paquete_recibido->data);
		int pid = pcb->programID;
		free(paquete_recibido);
	}

	return 0;
}

void iniciarCPU(){
	printf("%s", "\n\n====== INICIO CPU ======\n\n");

}
void cargarConfiguracion(char* pathconf){
	t_config* config = config_create(getenv("archivo_configuracion_CPU"));
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	printf("IP KERNEL: %s \n", ip_kernel);
	printf("PUERTO_KERNEL: %i \n", puerto_kernel);
	printf("IP MEMORIA: %s \n", ip_memoria);
	printf("PUERTO_MEMORIA: %i \n", puerto_memoria);
	printf("El archivo de configuracion fue cargado con exito\n");
}

//funcion que conecta CPU con Kernel utilizando sockets
int conectarConElKernel(){
	printf("Inicio de conexion con Kernel\n");
	// funcion deSockets
	kernel = conectar_a(ip_kernel,puerto_kernel);

	if (kernel==0){
		printf("CPU: No se pudo conectar con el Kernel\n");
		exit (EXIT_FAILURE);
	}
	printf("CPU: Kernel recibio nuestro pedido de conexion\n");

	printf("CPU: Iniciando Handshake\n");
	bool resultado = realizar_handshake(kernel,12);
	if (resultado){
		printf("Handshake exitoso! Conexion establecida\n");
		return kernel;
	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		exit (EXIT_FAILURE);
	}
}

//funcion que conecta CPU con Memoria utilizando sockets
int conectarConMemoria(){
	printf("Inicio de conexion con Memoria\n");
	// funcion deSockets
	memoria = conectar_a(ip_memoria,puerto_memoria);

	if (memoria==0){
		printf("CPU: No se pudo conectar con la Memoria\n");
		exit (EXIT_FAILURE);
	}
	printf("CPU: Memoria recibio nuestro pedido de conexion\n");

	printf("CPU: Iniciando Handshake\n");
	bool resultado = realizar_handshake(memoria,15);
	if (resultado){
		printf("Handshake exitoso! Conexion establecida\n");
		return memoria;
	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		exit (EXIT_FAILURE);
	}
}


//Funcion que toma lo que envio el Kernel y lo convierte en el PCB.
t_pcb* deserializarPCB(char* buffer){
	t_pcb* pcb;

	pcb = malloc(sizeof(t_pcb));
	memcpy(pcb, buffer, sizeof(t_pcb));
	buffer += sizeof(t_pcb);

	pcb->programID = malloc(sizeof(int));
	memcpy(pcb->programID, buffer, sizeof(int));
	buffer =+ sizeof(int);

	pcb->pageCounter = malloc(sizeof(int));
	memcpy(pcb->pageCounter, buffer, sizeof(int));
	buffer =+ sizeof(int);

	return pcb;
}
