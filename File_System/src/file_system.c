
#include "file_system.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

	logger = log_create("file_system.log","File_System",0,LOG_LEVEL_INFO);

	printf("%s", "\n\n====== INICIO FILE SYSTEM ======\n\n");
	cargarConfiguracion(argv[0]);

	pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	pthread_join(servidorConexionesKernel, NULL);

	return 0;
}


void cargarConfiguracion(char* pathconf) {

	log_info(logger,"Cargando archivo de configuracion 'file_system.config'...\n\n");
	t_config* config = config_create(getenv("archivo_configuracion"));
	puerto = config_get_int_value(config, "PUERTO");
	puntoMontaje = config_get_string_value(config, "PUNTO_MONTAJE");

	if(config_has_property(config,"PUERTO") && config_has_property(config,"PUNTO_MONTAJE"))
		log_info(logger,"Archivo de configuracion cargado correctamente");
	else {
		log_error(logger,"Error al cargar archivo de configuracion");
		if(!config_has_property(config,"PUERTO"))
			log_error(logger,"No esta setteado el PUERTO\n");
		if(!config_has_property(config,"PUNTO_MONTAJE"))
			log_error(logger,"Punto Montaje: %s \n",puntoMontaje);
	}
}


void* hiloServidorKernel(void *arg) {

	log_info(logger,"Hilo Servidor KERNEL\n");
	int socketCliente;
	int* socketClienteTemp;
	socketKernel = socket_escucha("127.0.0.1", puerto);
	log_info(logger,"Creacion socket servidor KERNEL exitosa\n\n");
	listen(socketKernel, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketKernel);
		log_info(logger,"Iniciando Handshake con KERNEL\n");
		bool resultado_hand = esperar_handshake(socketCliente);
		if(resultado_hand) {
			log_info(logger,"Conexión aceptada del KERNEL %d!!", socketCliente);
			printf("Conexion aceptada del KERNEL %d, esperando mensajes \n",socketCliente);
		}
		else {
			log_info(logger,"Handshake fallo, se aborta conexion\n\n");
			exit (EXIT_FAILURE);
		}
		socketClienteTemp = malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionKernel;
		pthread_create(&conexionKernel, NULL, hiloConexionKernel, (void*)socketClienteTemp);
	}

}

void* hiloConexionKernel(void* socket) {

	//File system espera recibir mensajes de kernel

	while(1) {
		char* buffer = malloc(1000);
		int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
		if (bytesRecibidos <= 0) {
			log_error(logger,"El proceso se desconecto\n");
			return 1;
		}
		buffer[bytesRecibidos] = '\0';
		log_info(logger,"Me llegaron %d bytes con %s, del KERNEL %d\n", bytesRecibidos, buffer,*(int*)socket);
		printf("Me llegaron %d bytes con %s, del KERNEL %d\n", bytesRecibidos, buffer,*(int*)socket);
		free(buffer);
	}

	return 0;
}
