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
	int i;
	cantidadDeProgramas = 0;

	//Crear Log
	logger = log_create("kernel.log","Kernel",0,LOG_LEVEL_INFO);

	cargarConfiguracion();
	mostrarConfiguracion();

	prepararSocketsServidores();

	while(1){
		//elimina todos los clientes que hayan cerrado conexion
		compactaClaves(socketCliente, &numeroClientes);

		//se inicia descriptor lectura
		FD_ZERO (&fds_activos);

		//se agrega para select el servidor
		FD_SET (socketCPU, &fds_activos);
		FD_SET (socketConsola, &fds_activos);

		//se agregan para el select los clientes ya conectados
		for (i=0; i<numeroClientes; i++)
			FD_SET (socketCliente[i],&fds_activos);

		//el valor del descriptor mas grande, si no hay, retorna 0
		socketMasGrande = dameSocketMasGrande(socketCliente, numeroClientes);

		if(socketMasGrande < socketConsola){
			socketMasGrande = socketConsola;
		}

		//esperamos hasta que haya un cambio
		printf("Antes Select\n");
		select(socketMasGrande +1, &fds_activos, NULL, NULL, NULL);
		printf("Hubo cambios en Sockets, verificando...\n\n");

		/* Se comprueba si algún cliente ya conectado ha enviado algo */
		for (i=0; i<numeroClientes; i++){
			printf("verificando cliente %d \n", i+1);
			if(FD_ISSET(socketCliente[i],&fds_activos)){
				paqueteRecibido = recibir(socketCliente[i]);
				//verifica codigo de operacion, si es -1 se desconecto
				if(paqueteRecibido->codigo_operacion > 0){
					printf("Me envio datos\n");
					procesarPaqueteRecibido(paqueteRecibido);
				}
				else {
					printf("El cliente %d se desconecto\n",i+1);
					log_info(logger, "El cliente %d se desconecto\n",i+1);
					socketCliente[i] = -1;
				}

			}
			else printf("No hubo cambios en cliente %d, continua \n\n",i+1);
		}
		verSiHayNuevosClientes();
	}
	return EXIT_SUCCESS;
}

void cargarConfiguracion() {

	printf("Cargando archivo de configuracion 'kernel.config'\n\n");
	log_info(logger, "Cargando archivo de configuracion 'kernel.config'");

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
	//sem_inits = config_get_array_value(config, "SEM_INIT");
	//shared_vars = config_get_array_value(config, "SHARED_VARS");
	stack_size = config_get_int_value(config, "STACK_SIZE");

	log_info(logger, "Archivo de configuración cargado");
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

void prepararSocketsServidores(){
	socketCPU = socket_escucha("127.0.0.1",puerto_cpu);
	listen(socketCPU,1);
	socketConsola = socket_escucha("127.0.0.1",puerto_prog);
	listen(socketConsola,1);
	printf("Estoy Escuchando\n");
	log_info(logger, "Sockets escuchando");
}

void verSiHayNuevosClientes(){
	if(FD_ISSET (socketCPU, &fds_activos)){
		printf("Nuevo pedido de conexion CPU\n");
		log_info(logger, "Nuevo pedido de conexion CPU");
		nuevoCliente(socketCPU, socketCliente, &numeroClientes);
	}
	if(FD_ISSET (socketConsola, &fds_activos)){
		printf("Nuevo pedido de conexion Consola\n");
		log_info(logger, "Nuevo pedido de conexion Consola");
		nuevoCliente(socketConsola, socketCliente, &numeroClientes);
	}
}

void nuevoCliente (int servidor, int *clientes, int *nClientes)
{
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes] = aceptar_conexion(servidor);
	(*nClientes)++;

	/* Si se ha superado el maximo de clientes, se cierra la conexión,
	 * se deja todo como estaba y se vuelve. */

	if ((*nClientes) > MAX_CLIENTES) {
		close (clientes[(*nClientes) -1]);
		(*nClientes)--;
		return;
	}

	bool resultado_CPU = esperar_handshake(clientes[*nClientes - 1], 12);
	bool resultado_Consola = esperar_handshake(clientes[*nClientes - 1], 11);
	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	if(resultado_CPU || resultado_Consola){
		log_info(logger, "Handshake OK, pedido de conexion aceptado");
		printf ("Aceptado cliente %d\n", *nClientes);
	}
	else{
		log_error(logger, "Handshake fallo, pedido de conexion rechazado");
		printf ("Handshake rechazado\n");
	}

	return;
}

void procesarPaqueteRecibido(t_paquete* paqueteRecibido){
	switch(paqueteRecibido->codigo_operacion){
		//Codigo 101: Ejecutar Script Ansisop
		case 101:
			crearProcesoAnsisop();
			break;
		default:
			break;
	}
}

void crearProcesoAnsisop(){
	//Creo PCB para el proceso en cuestion
	t_pcb* pcb = malloc(sizeof(t_pcb));
	cantidadDeProgramas++;
	pcb->pid = cantidadDeProgramas;


	//Pido paginas a memoria y memoria me dice si le alcanzan
	int resultadoPedidoPaginas = pedirPaginasParaProceso(cantidadDeProgramas);


	//Si la memoria devolvio OK, asigno la cantidad de paginas al PCB. Si no devolvió OK, ????
	if(resultadoPedidoPaginas > 0){
		pcb->pageCounter = resultadoPedidoPaginas;

		//TODO El programa ya esta listo para ejecutarse, ver tema de planificación
	}
	else {
		//????
	}
}

int pedirPaginasParaProceso(int pid){
	memoria = conectarConLaMemoria();
	//Calculo paginas de memoria que necesito pedir de memoria para este script
	int paginasAPedir = ceil(paqueteRecibido->tamanio/TAMANIODEPAGINA);

	//ENUMS, NADA DE CODIGOS!
	t_pedidoDePaginas* pedidoDePaginas = malloc(sizeof(t_pedidoDePaginas));
	pedidoDePaginas->pid = pid;
	pedidoDePaginas->paginasAPedir = paginasAPedir;

	//Armo el paquete con el pedido de paginas para mandar a memoria y lo envio
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = 201; //HAY QUE HACER UN ARCHIVO DE CODIGOS URGENTE, ESTE LO INVENTE 201 = PEDIDO DE PAGINAS DE KERNEL A MEMORIA
	paquete->tamanio = sizeof(t_pedidoDePaginas);
	paquete->data = pedidoDePaginas;

	enviar(memoria, 201, paquete->tamanio,paquete);//VA ACA EL 201 O EN EL PAQUETE?

	free(pedidoDePaginas);
	free(paquete);

	//Tengo que esperar a que vuelva la respuesta del pedido. Si esta OK, devuelve la cantidad de paginas, sino devuelve -1
	t_paquete* respuestaAPedidoDePaginas;
	respuestaAPedidoDePaginas = recibir(memoria);

	return respuestaAPedidoDePaginas->data;
}

int conectarConLaMemoria(){
	printf("Inicio de conexion con la Memoria\n");
	log_info(logger, "Inicio de conexion con la Memoria");
	memoria = conectar_a(ip_memoria,puerto_memoria);

	if (memoria==0){
		printf("No se pudo conectar con la Memoria\n");
		log_error(logger, "No se pudo conectar con la Memoria");
		exit (EXIT_FAILURE);
	}

	printf("MEMORIA: Recibio pedido de conexion de Kernel\n");
	log_info(logger, "MEMORIA: Recibio pedido de conexion de Kernel");

	printf("MEMORIA: Iniciando Handshake\n");
	bool resultado = realizar_handshake(memoria, 13);
	if (resultado){
		printf("MEMORIA: Handshake exitoso! Conexion establecida\n");
		log_info(logger, "MEMORIA: Handshake exitoso! Conexion establecida");
		return memoria;
	}
	else{
		printf("MEMORIA: Fallo en el handshake, se aborta conexion\n");
		log_error(logger, "MEMORIA: Fallo en el handshake, se aborta conexion");
		exit (EXIT_FAILURE);
	}
}

/*
 * Función que devuelve el valor máximo en la tabla.
 * Supone que los valores válidos de la tabla son positivos y mayores que 0.
 * Devuelve 0 si n es 0 o la tabla es NULL */
int dameSocketMasGrande (int *tabla, int n) {
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

void compactaClaves (int *tabla, int *n) {
	int i,j;

	if ((tabla == NULL) || ((*n) == 0))
		return;

	j=0;
	for (i=0; i<(*n); i++) {
		if (tabla[i] != -1) {
			tabla[j] = tabla[i];
			j++;
		}
	}

	*n = j;
}

