
#include "file_system.h"

int main() {

	crearArchivoLog();
	cargarConfiguracion();

	fileSystemServer = socket_escucha(ipFileSystem,puerto);
	listen(fileSystemServer, 1);
	log_info(logger,"Socket %d creado y escuchando", socketKernel);
	socketKernel = aceptar_conexion(fileSystemServer);

	bool resultado = esperar_handshake(socketKernel,KernelValidacion);

	if(resultado){
		log_info(logger,"Conexión aceptada del KERNEL %d!!", socketKernel);
		printf("Conexion aceptada del KERNEL %d \n",socketKernel);
	}
	else
	{
		log_info(logger,"Handshake fallo, se aborta conexion\n");
		printf("Conexion abortada\n");
		exit (EXIT_FAILURE);
	}
	while(1){
		//Aca tenemos que recibir pedidos de Kernel, se puede hacer switch de codigo de operacion
		paquete = recibir(socketKernel);
	}


	//Variante HILOS
	//pthread_create(&servidorConexionesKernel, NULL, hiloServidorKernel, NULL);
	//pthread_join(servidorConexionesKernel, NULL);

	return 0;
}

void crearArchivoLog(){
		logger = iniciarLog(ARCHIVOLOG,"File_System");
		log_info(logger, "Iniciando File_System. \n");
	}

void cargarConfiguracion() {

	log_info(logger,"Cargando archivo de configuracion 'file_system.config'...\n");
	t_config* config = config_create(getenv("archivo_configuracion_fs"));
	puerto = config_get_int_value(config, "PUERTO");
	puntoMontaje = config_get_string_value(config, "PUNTO_MONTAJE");

	if(config_has_property(config,"PUERTO") && config_has_property(config,"PUNTO_MONTAJE"))
		log_info(logger,"Archivo de configuracion cargado correctamente\n");
	else {
		log_error(logger,"Error al cargar archivo de configuracion");
		if(!config_has_property(config,"PUERTO"))
			log_error(logger,"No esta setteado el PUERTO\n");
		if(!config_has_property(config,"PUNTO_MONTAJE"))
			log_error(logger,"Punto Montaje: %s \n",puntoMontaje);
	}
}

/*
void* hiloServidorKernel(void *arg) {

	log_info(logger,"Hilo Servidor KERNEL\n");
	int socketCliente;
	int* socketClienteTemp;
	socketKernel = socket_escucha("127.0.0.1", puerto);
	log_info(logger,"Creacion socket servidor KERNEL exitosa\n");
	listen(socketKernel, 1024);
	while(1) {
		socketCliente = aceptar_conexion(socketKernel);
		log_info(logger,"Iniciando Handshake con KERNEL\n");
		bool resultado_hand = esperar_handshake(socketCliente,KernelValidacion);
		if(resultado_hand) {
			log_info(logger,"Conexión aceptada del KERNEL %d!!", socketCliente);
			printf("Conexion aceptada del KERNEL %d, esperando mensajes \n",socketCliente);
		}
		else {
			log_info(logger,"Handshake fallo, se aborta conexion\n");
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
*/
