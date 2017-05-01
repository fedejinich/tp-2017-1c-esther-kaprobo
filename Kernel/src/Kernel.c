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
	fd_set descriptoresLectura;
	int socketServidor ;
	int socketCliente[MAX_CLIENTES];
	int numeroClientes = 0;
	int maximo;
	int i;




	logger = log_create("kernel.log","Kernel",0,LOG_LEVEL_INFO);

	cargarConfiguracion();
	mostrarConfiguracion();

	socketServidor = socket_escucha("127.0.0.1",puerto_prog);
	listen(socketServidor,1);
	printf("Estoy Escuchando\n");
	t_paquete* paqueteRecibido;

	while(1){
			//elimina todos los clientes que hayan cerrado conexion
			compactaClaves(socketCliente, &numeroClientes);

			//se inicia descriptor lectura
			FD_ZERO (&descriptoresLectura);

			//se agrega para select el servidor
			FD_SET (socketServidor, &descriptoresLectura);

			//se agregan para el select los clientes ya conectados
			for (i=0; i<numeroClientes; i++)
				FD_SET (socketCliente[i],&descriptoresLectura);

			//el valor del descriptor mas grande, si no hay, retorna 0
			maximo = dameMaximo(socketCliente, numeroClientes);

			if(maximo < socketServidor)
				maximo = socketServidor;

			//esperamos hasta que haya un cambio
			printf("Antes Select\n");
			select(maximo +1, &descriptoresLectura, NULL, NULL, NULL);
			printf("Hubo cambios en Sockets, verificando...\n\n");

			/* Se comprueba si algún cliente ya conectado ha enviado algo */
			for (i=0; i<numeroClientes; i++){
				printf("verificando cliente %d \n", i+1);
				if(FD_ISSET(socketCliente[i],&descriptoresLectura)){
					paqueteRecibido = recibir(socketCliente[i]);
					//verifica codigo de operacion, si es -1 se desconecto
					if(paqueteRecibido->codigo_operacion >0){
						printf("me envio datos\n");
					}
					else {
						printf("el cliente %d se desconecto\n",i+1);
						socketCliente[i] = -1;
					}

				}
				else printf("No hubo cambios en cliente %d, continua \n\n",i+1);
			}
			//comprueba si algun cliente nuevo
			if(FD_ISSET (socketServidor, &descriptoresLectura)){
				printf("Nuevo pedido de conexion\n");
				nuevoCliente(socketServidor, socketCliente, &numeroClientes);
			}
		}
	return EXIT_SUCCESS;
}

void cargarConfiguracion() {

	printf("Cargando archivo de configuracion 'kernel.config'\n\n");
	t_config* config = config_create(getenv("archivo_configuracion_kernel"));
	puerto_prog = config_get_int_value(config, "PUERTO_PROG");
	puerto_cpu = config_get_int_value(config, "PUERTO_CPU");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
	ip_fs = config_get_string_value(config, "IP_FS");
	puerto_fs = config_get_int_value(config, "PUERTO_FS");
	quantum = config_get_int_value(config, "QUANTUM");
	quantum_sleep = config_get_int_value(config, "QUANTUM_SLEEP");
	algoritmo = config_get_string_value(config, "ALGORITMO");
	grado_multiprog = config_get_int_value(config, "GRADO_MULTIPROG");
	//sem_ids = config_get_array_value(config, "SEM_IDS");
	//sem_inits = config_get_array_value(config, "SEM_INIT");;
	//shared_vars = config_get_array_value(config, "SHARED_VARS");;
	stack_size = config_get_int_value(config, "STACK_SIZE");;
}

void mostrarConfiguracion(){
	printf("Puerto Prog: %i \n", puerto_prog);
	printf("Puerto CPU: %i \n", puerto_cpu);
	printf("IP Memoria: %s \n", ip_memoria);
	printf("Puerto Memoria: %i \n", puerto_memoria);
	printf("IP File System: %s \n", ip_fs);
	printf("Puerto File System: %i \n", puerto_fs);
	printf("Quantum: %i \n", quantum);
	printf("Quantum Sleep: %i \n", quantum_sleep);
	printf("Algoritmo: %s \n", algoritmo);
	printf("Grado Multiprogramacion: %i \n", grado_multiprog);

	//Falta mostrar arrays

	printf("Stack Size: %i \n", stack_size);
}

void nuevoCliente (int servidor, int *clientes, int *nClientes)
{
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes] = aceptar_conexion(servidor);
	(*nClientes)++;

	/* Si se ha superado el maximo de clientes, se cierra la conexión,
	 * se deja todo como estaba y se vuelve. */

	printf ("CANTIDAD DE CLIENTES %d\n", *nClientes);
	printf ("MAXIMO DE CLIENTES %i\n", MAX_CLIENTES);
	if ((*nClientes) > MAX_CLIENTES)
	{
		close (clientes[(*nClientes) -1]);
		(*nClientes)--;
		return;
	}

	bool resultado = esperar_handshake(clientes[*nClientes - 1], 11) || esperar_handshake(clientes[*nClientes - 1], 12);

	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	if(resultado){
		printf ("Aceptado cliente %d\n", *nClientes);
	}
	else{
		printf ("Handshake rechazado\n");
	}


	return;
}

/*
 * Función que devuelve el valor máximo en la tabla.
 * Supone que los valores válidos de la tabla son positivos y mayores que 0.
 * Devuelve 0 si n es 0 o la tabla es NULL */
int dameMaximo (int *tabla, int n)
{
	int i;
	int max;

	if ((tabla == NULL) || (n<1))
		return 0;

	max = tabla[0];
	for (i=0; i<n; i++)
		if (tabla[i] > max)
			max = tabla[i];

	return max;
}

void compactaClaves (int *tabla, int *n)
{
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++)
	{
		if (tabla[i] != -1)
		{
			tabla[j] = tabla[i];
			j++;
		}
	}

	*n = j;
}

