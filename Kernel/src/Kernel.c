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

	pthread_create(&servidorConexionesConsola, NULL, hiloServidorConsola, NULL);
	pthread_create(&servidorConexionesCPU, NULL, hiloServidorCPU, NULL);
	pthread_join(servidorConexionesConsola, NULL);
	pthread_join(servidorConexionesCPU, NULL);


	/*servidor = inicializarServidor();
	prepararservidoretServidorParaEscuchar();
	atenderYCrearConexiones();
	*/
	return EXIT_SUCCESS;
}

void cargarConfiguracion() {

	printf("Cargando archivo de configuracion 'kernel.config'\n\n");
	t_config* config = config_create(getenv("archivo_configuracion"));
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
	//Falta mostrar arrays
}


void *hiloServidorConsola(void *arg){
	printf("------Hilo CONSOLA------\n");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketConsola = socket_escucha("127.0.0.1", puerto_prog);
	printf("Creacion socket servidor consola exitosa\n\n");
	listen(socketConsola, 1024);
	while(1){

		//printf("a: %d\n",a);
		socketCliente = aceptar_conexion(socketConsola);
		printf("Iniciando Handshake con CONSOLA\n");
			bool resultado_hand = esperar_handshake(socketCliente);
			if(resultado_hand){
				printf("Conexión aceptada de la consola %d!!\n\n", socketCliente);


			}else {
				printf("Handshake fallo, se aborta conexion\n\n");
				exit (EXIT_FAILURE);
			};
		socketClienteTemp=malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionConsola;
		pthread_create(&conexionConsola, NULL, hiloConexionConsola, (void*)socketClienteTemp);

	}

}

void *hiloConexionConsola(void *socket){

	while(1){
	char* buffer = malloc(1000);
	int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
	if (bytesRecibidos <= 0) {
		perror("El proceso se desconecto\n");
		return 1;
	}

	buffer[bytesRecibidos] = '\0';
	printf("Me llegaron %d bytes con %s, de la consola %d\n", bytesRecibidos, buffer,*(int*)socket);
	send(*(int*)socket,*buffer,strlen(buffer),0);
	free(buffer);
	}


	return 0;

}

void *hiloServidorCPU(void *arg){
	printf("------Hilo CPU------\n");
	int servidorSocket, socketCliente;
	int *socketClienteTemp;
	socketCPU = socket_escucha("127.0.0.1", puerto_cpu);
	printf("Creacion socket servidor CPU exitosa\n\n");
	listen(socketCPU, 1024);
	while(1){


		socketCliente = aceptar_conexion(socketCPU);
		printf("Iniciando Handshake con CPU\n");
			bool resultado_hand = esperar_handshake(socketCliente);
			if(resultado_hand){
				printf("Conexión aceptada de la CPU %d!!\n\n", socketCliente);


			}else {
				printf("Handshake fallo, se aborta conexion\n\n");
				exit (EXIT_FAILURE);
			};
		socketClienteTemp=malloc(sizeof(int));
		*socketClienteTemp = socketCliente;
		pthread_t conexionCPU;
		pthread_create(&conexionCPU, NULL, hiloConexionCPU, (void*)socketClienteTemp);

	}

}

void *hiloConexionCPU(void *socket){

	while(1){
	char* buffer = malloc(1000);
	int bytesRecibidos = recv(*(int*)socket, buffer, 1000, 0);
	if (bytesRecibidos <= 0) {
		perror("El proceso se desconecto\n");
		return 1;
	}

	buffer[bytesRecibidos] = '\0';
	printf("Me llegaron %d bytes con %s, de la CPU %d\n", bytesRecibidos, buffer,*(int*)socket);
	free(buffer);
	}

	return 0;

}




int inicializarServidor(){
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(8080);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		perror("Falló el bind");
		return 1;
	}

	printf("Estoy escuchando\n");
	listen(servidor, 100);
	return servidor;
}

void prepararservidoretServidorParaEscuchar(){
	FD_ZERO(&fds_activos);
	FD_SET(servidor, &fds_activos);
}

void atenderYCrearConexiones(){
	while (1)
	{
		/* Bloquea hasta que llegan modificaciones de uno o mas sockets. */
		timeout.tv_sec = 10;
		timeout.tv_usec = 500000;
		int sockets_a_atender = select (FD_SETSIZE, &fds_activos, NULL, NULL, &timeout);
		if (sockets_a_atender < 0)
		{
			perror ("Ocurrio un error en el select");
			exit (EXIT_FAILURE);
		}

		if (sockets_a_atender == 0)
		{
			perror ("Ocurrio un error de timeout");
			exit (EXIT_FAILURE);
		}

		// Atender a todos los sockets con alguna modificacion.
		int un_socket;
		struct sockaddr_in cliente;
		size_t size;

		//Recorro el listado de sockets para ver cuales fueron modificados.
		for (un_socket = 0; un_socket < FD_SETSIZE; ++un_socket){
			//Si un socket ISSET, significa que fue modificado, sino, no
			if (FD_ISSET (un_socket, &fds_activos))
			{
				//Si el socket modificado en cuestion es el servidor, significa que hay un pedido de nueva conexion
				if (un_socket == servidor)
				{
					int socket_nueva_conexion;
					size = sizeof (cliente);
					socket_nueva_conexion = accept (servidor, (struct servidoraddr *) &cliente, &size);
					if (socket_nueva_conexion < 0)
					{
						perror ("Ocurrio un error en el accept");
						exit (EXIT_FAILURE);
					}
					fprintf (stderr, "Servidor: conectar desde host %s, puerto %hd.\n", inet_ntoa (cliente.sin_addr), ntohs (cliente.sin_port));

					//Agrego el socket de la nueva conexion al listado de sockets a monitorear
					FD_SET (socket_nueva_conexion, &fds_activos);
				}
				else
				{
					//Es un socket que tiene informacion y hay que leerlo
					char* mensaje = recibirMensajeCliente(un_socket);
					printf("%s \n" , mensaje);
				}
			}
		}
	}
}

char* recibirMensajeCliente(int un_socket){

	char* payload;
	recv(un_socket, &payload, 512, 0);

	//Para cuando utilicemos el header, por ahora, basico
	/*int header;

	recv(un_socket, &header, sizeof(int), 0);

	recv(un_socket, &payload, tamanio_payload, 0);

	return payload;*/
}


