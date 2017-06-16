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

	inicializar();

	while(1){
		manejarSockets();
		verSiHayNuevosClientes();
	}
	return EXIT_SUCCESS;
}

void inicializar(){
	cantidadDeProgramas = 0;

	list_create(colaNew);
	list_create(colaReady);
	list_create(colaExec);
	list_create(colaExit);

	//Crear Log
	logger = log_create("kernel.log","Kernel",0,LOG_LEVEL_INFO);

	cargarConfiguracion();
	//mostrarConfiguracion();

	prepararSocketsServidores();

	memoria = conectarConLaMemoria();

	fileSystem = conectarConFileSystem();
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

void manejarSockets(){
	//elimina todos los clientes que hayan cerrado conexion
	compactaClaves(socketCliente, &numeroClientes);

	//se inicia descriptor lectura
	FD_ZERO (&fds_activos);

	//se agrega para select el servidor
	FD_SET (socketCPU, &fds_activos);
	FD_SET (socketConsola, &fds_activos);

	//se agregan para el select los clientes ya conectados
	for (int i=0; i<numeroClientes; i++)
		FD_SET (socketCliente[i],&fds_activos);

	//el valor del descriptor mas grande, si no hay, retorna 0
	socketMasGrande = dameSocketMasGrande(socketCliente, numeroClientes);

	if(socketMasGrande < socketConsola){
		socketMasGrande = socketConsola;
	}

	//esperamos hasta que haya un cambio
	select(socketMasGrande +1, &fds_activos, NULL, NULL, NULL);

	/* Se comprueba si algún cliente ya conectado ha enviado algo */
	for (i=0; i<numeroClientes; i++){
		log_info(logger, "Verificando cliente %d", i+1);
		if(FD_ISSET(socketCliente[i],&fds_activos)){
			paqueteRecibido = recibir(socketCliente[i]);

			//verifica codigo de operacion, si es -1 se desconecto
			if(paqueteRecibido->codigo_operacion > 0){
				log_info(logger, "Me envio datos el socket %i", socketCliente[i]);
				procesarPaqueteRecibido(paqueteRecibido, socketCliente[i]);
			}
			else {
				log_info(logger, "El cliente %d se desconecto",i+1);
				socketCliente[i] = -1;
			}

		}
		else log_info(logger, "No hubo cambios en cliente %d, continua",i+1);
	}
}

void verSiHayNuevosClientes(){
	if(FD_ISSET (socketCPU, &fds_activos)){
		log_info(logger, "Nuevo pedido de conexion CPU");
		nuevoClienteCPU(socketCPU, socketCliente, &numeroClientes);
	}
	if(FD_ISSET (socketConsola, &fds_activos)){
		log_info(logger, "Nuevo pedido de conexion Consola");
		nuevoClienteConsola(socketConsola, socketCliente, &numeroClientes);
	}
}

void nuevoClienteCPU (int servidor, int *clientes, int *nClientes)
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

	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	if(resultado_CPU){
		log_info(logger, "Handshake OK, pedido de conexion cliente %d aceptado", *nClientes);
		cpusConectadas[indiceCPUsConectadas] = servidor;
		indiceCPUsConectadas++;
		cpusDisponibles[indiceCPUsDisponibles] = servidor;
		indiceCPUsDisponibles++;
		intentarMandarProcesoACPU();
	}
	else{
		log_error(logger, "Handshake fallo, pedido de conexion rechazado");
		printf ("Handshake rechazado\n");
	}

	return;
}

void nuevoClienteConsola (int servidor, int *clientes, int *nClientes)
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

	bool resultado_Consola = esperar_handshake(clientes[*nClientes - 1], 11);
	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	if(resultado_Consola){
		log_info(logger, "Handshake OK, pedido de conexion cliente %d aceptado", *nClientes);
	}
	else{
		log_error(logger, "Handshake fallo, pedido de conexion cliente %d rechazado", *nClientes);
	}

	return;
}

void procesarPaqueteRecibido(t_paquete* paqueteRecibido, un_socket socketActivo){
	switch(paqueteRecibido->codigo_operacion){
		//Codigo 101: Crear Script Ansisop
		case 101:
			crearProcesoAnsisop(socketActivo);
			break;
		case 1000000: //Codigo a definir que indica fin de proceso en CPU y libero
			finalizarProcesoCPU(paqueteRecibido, socketActivo);
			break;
		default:
			break;
	}
}

void crearProcesoAnsisop(un_socket socketQueMandoElProceso){
	log_info(logger, "Creando nuevo proceso Ansisop");

	t_proceso* nuevoProcesoAnsisop = malloc(sizeof(nuevoProcesoAnsisop));

	//Creo PCB para el proceso en cuestion
	log_info(logger, "Creando PCB del nuevo proceso Ansisop");
	t_pcb* pcb = malloc(sizeof(t_pcb));
	cantidadDeProgramas++;
	nuevoProcesoAnsisop->pcb = pcb;
	nuevoProcesoAnsisop->pcb->pid = cantidadDeProgramas;
	nuevoProcesoAnsisop->socketConsola = socketQueMandoElProceso;

	//Envio a consola el PID del nuevo proceso
	//EnvioPIDDeNuevoProceso 107
	enviar(nuevoProcesoAnsisop->socketConsola, 107, sizeof(int), nuevoProcesoAnsisop->pcb->pid);

	pasarProcesoALista(nuevoProcesoAnsisop->pcb, ListaNew, ListaNull);

	if(grado_multiprog < cantidadDeProgramas){
		//SI PERMITE, PASA DE NEW A READY
		//Pido paginas a memoria y memoria me dice si le alcanzan
		int resultadoPedidoPaginas = pedirPaginasParaProceso(cantidadDeProgramas);

		//Si la memoria devolvio OK, asigno la cantidad de paginas al PCB. Si no devolvió OK, lo finalizo con error por falta de memoria
		if(resultadoPedidoPaginas > 0){
			pcb->pageCounter = resultadoPedidoPaginas;

			pasarProcesoAlista(nuevoProcesoAnsisop->pcb, ListaReady, ListaNew);

			intentarMandarProcesoACPU(nuevoProcesoAnsisop->pcb);
		}
		else {
			//FALTA ELIMINAR DE NEW
			finalizarProceso(nuevoProcesoAnsisop,-1);
		}
	}

}

void finalizarProcesoCPU(t_paquete* paquete, un_socket socketCPU){
	//Desarmar paquete para ver codigo de finalizacion
	int pid = 0; //CAMBIAR POR PID DEL PAQUETE
	int codigoDeFinalizacion = 0; //CAMBIAR POR CODIGO DEL PAQUETE
	finalizarProceso(pid, codigoDeFinalizacion, ListaExec);
	intentarMandarProcesoACPU();
}

void finalizarProceso(t_proceso* procesoAFinalizar, int exitCode, int listaDondeEstaba){
	procesoAFinalizar->pcb->exitCode = exitCode;
	pasarProcesoALista(procesoAFinalizar->pcb, ListaExit, listaDondeEstaba);
	int codigoDeFinalizacion;
	if(exitCode == 0){
		//Codigo de Finzalicacion correcta del proceso
		codigoDeFinalizacion = 102;
	}
	else{
		//Codigo Kernel Aborto Proceso
		codigoDeFinalizacion = 106;
	}

	enviar(procesoAFinalizar->socketConsola, codigoDeFinalizacion, sizeof(int), NULL);
}

int pedirPaginasParaProceso(int pid){
	//Calculo paginas de memoria que necesito pedir de memoria para este script
	int paginasAPedir = ceil(paqueteRecibido->tamanio/TAMANIODEPAGINA);

	t_pedidoDePaginasKernel* pedidoDePaginas = malloc(sizeof(t_pedidoDePaginasKernel));
	pedidoDePaginas->pid = pid;
	pedidoDePaginas->paginasAPedir = paginasAPedir;

	//Envio a memoria el pedido de pagina
	enviar(memoria, 201, sizeof(t_pedidoDePaginasKernel),pedidoDePaginas);//VA ACA EL 201 O EN EL PAQUETE?

	free(pedidoDePaginas);

	//Tengo que esperar a que vuelva la respuesta del pedido. Si esta OK, devuelve la cantidad de paginas, sino devuelve -1
	t_paquete* respuestaAPedidoDePaginas;
	respuestaAPedidoDePaginas = recibir(memoria);

	return respuestaAPedidoDePaginas->data;
}

void intentarMandarProcesoACPU(int pid){
	//SI HAY CPUS DISPONIBLES, Y TENGO ALGUN PROCESO EN LA COLA DE READY, MANDO UN PROCESO A CPU
	if(){
		//Elijo una CPU del listado de CPUs disponibles y la saco del listado de disponibles
		un_socket socketCPU = cpusDisponibles[indiceCPUsDisponibles];
		cpusDisponibles[indiceCPUsDisponibles] = -1;
		indiceCPUsDisponibles--;

		//Evaluacion del algoritmo de planficacion a utilizar
		if(){
			//FIFO
			int pid = 0; //ACA EN REALIDAD VIENE UN POP DE LA COLA DE READY, VER SI RODRI SOLUCIONO
			enviarUnProcesoACPU(socketCPU, pid);
		}
		else{
			//ROUND ROBIN

		}

	}
}

void enviarUnProcesoACPU(un_socket socketCPU, int pid){
	//PASO EL PROCESO A EXEC Y LO MANDO A LA CPU CORRESPONDIENTE

	t_proceso* proceso = buscarProcesoEnReadySegunPID(pid);

	pasarProcesoALista(proceso, ListaExec, ListaReady);

	//51651 VER CODIGO DE PROCESO A EJECUTAR EN CPU
	enviar(socketCPU, 51651, sizeof(t_proceso), proceso);
}

t_proceso buscarProcesoEnReadySegunPID(int pid){
	t_proceso* proceso = malloc(sizeof(t_proceso));

	//BUSCAR EN LISTA READY SEGUN PID

	return proceso;
}

int conectarConLaMemoria(){
	log_info(logger, "MEMORIA: Inicio de conexion");
	un_socket socketMemoria = conectar_a(ip_memoria, puerto_memoria);

	if (socketMemoria == 0){
		log_error(logger, "MEMORIA: No se pudo conectar");
		exit (EXIT_FAILURE);
	}

	log_info(logger, "MEMORIA: Recibio pedido de conexion de Kernel");

	log_info(logger, "MEMORIA: Iniciando Handshake");
	bool resultado = realizar_handshake(socketMemoria , 13);
	if (resultado){
		log_info(logger, "MEMORIA: Handshake exitoso! Conexion establecida");
		return socketMemoria ;
	}
	else{
		log_error(logger, "MEMORIA: Fallo en el handshake, se aborta conexion");
		exit (EXIT_FAILURE);
	}
}

int conectarConFileSystem(){
	log_info(logger, "FILESYSTEM: Inicio de conexion");
	un_socket socketFileSystem = conectar_a(ip_fs, puerto_fs);

	if (socketFileSystem == 0){
		log_error(logger, "No se pudo conectar con la Memoria");
		exit (EXIT_FAILURE);
	}

	log_info(logger, "FILESYSTEM: Recibio pedido de conexion de Kernel");

	log_info(logger, "FILESYSTEM: Iniciando Handshake");
	bool resultado = realizar_handshake(socketFileSystem, 13);
	if (resultado){
		log_info(logger, "FILESYSTEM: Handshake exitoso! Conexion establecida");
		return socketFileSystem;
	}
	else{
		log_error(logger, "FILESYSTEM: Fallo en el handshake, se aborta conexion");
		exit (EXIT_FAILURE);
	}
}

void pasarProcesoALista(t_proceso* procesoAMover, int listaAMover, int listaAEliminar){
	switch (listaAMover){
		case ListaNew:
			list_add(colaNew,procesoAMover->pcb);
			break;
		case ListaReady:
			list_add(colaReady,procesoAMover->pcb);
			break;
		case ListaExec:
			list_add(colaExec,procesoAMover->pcb);
			break;
		case ListaExit:
			list_add(colaExit,procesoAMover->pcb);
			break;
	}

	//ESTO O VER COMO ELIMINAR DE LA LISTA DONDE ESTABA, SI ES CON PUSH/POP O QUE
	switch (listaAEliminar){
		case ListaNew:

			break;
		case ListaReady:

			break;
		case ListaExec:

			break;
		case ListaExit:

			break;
		case ListaNull:
			//No hay que eliminarla de ninguna lista, nuevo proceso
			break;
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
