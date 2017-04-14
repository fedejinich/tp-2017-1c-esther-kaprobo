
#include "file_system.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

	printf("%s", "\n\n====== INICIO FILE SYSTEM ======\n\n");
	cargarConfiguracion(argv[0]);

	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	pthread_join(servidorConexionesKernel, NULL);

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
		printf("No esta setteado el PUERTO\n");
	if(config_has_property(config,"PUNTO_MONTAJE"))
		printf("Punto Montaje: %s \n",puntoMontaje);
	else
		printf("No esta setteado el PUNTO_MONTAJE\n");

	printf("\n");
}


void* hiloServidorKernel(void *arg) {

	printf("------Hilo Servidor KERNEL------\n");
	int socketCliente;
	int* socketClienteTemp;
	socketKernel = socket_escucha("127.0.0.1", puerto);
	printf("Creacion socket servidor KERNEL exitosa\n\n");
	listen(socketKernel, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketKernel);
		printf("Iniciando Handshake con KERNEL\n");
		bool resultado_hand = esperar_handshake(socketCliente);
		if(resultado_hand)
			printf("Conexión aceptada del KERNEL %d!!\n\n", socketCliente);
		else {
			printf("Handshake fallo, se aborta conexion\n\n");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionKernel;
		pthread_create(&conexionKernel, NULL, hiloConexionKernel(socketClienteTemp), (void*)socketClienteTemp);
		//pthread_join(conexionKernel,NULL);
	}

}

void* hiloConexionKernel(un_socket socket) {

	while(1) {
		char* buffer = malloc(1000);
		int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			perror("El proceso se desconecto\n");
			return 1;
		}
		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d bytes con %s, del KERNEL %d\n", bytesRecibidos, buffer,*(int*)socket);
		free(buffer);
	}

	return 0;
}
