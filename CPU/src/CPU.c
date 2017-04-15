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
	kernel = conectarConElKernel();
	while (1){
			char mensaje[1000];
			scanf("%s", mensaje);
			send(kernel,mensaje,strlen(mensaje),0);
			char* buffer = malloc(1000);
				int bytesRecibidos = recv(kernel, buffer, 1000, 0);
				if (bytesRecibidos <= 0) {
					perror("El proceso se desconecto\n");
					return 1;
				}

				buffer[bytesRecibidos] = '\0';
				printf("Me llegaron %d bytes con %s, del kernel %d\n", bytesRecibidos, buffer,kernel);
		}
		return 0;
}

void iniciarCPU(){
	printf("%s", "\n\n====== INICIO CPU ======\n\n");

}
void cargarConfiguracion(char* pathconf){
	t_config* config = config_create(getenv("archivo_configuracion"));
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
	bool resultado = realizar_handshake(kernel);
	if (resultado){
		printf("Handshake exitoso! Conexion establecida\n");
		return kernel;
	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		exit (EXIT_FAILURE);
	}
}
