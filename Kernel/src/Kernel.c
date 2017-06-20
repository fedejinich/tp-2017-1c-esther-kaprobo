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
	pthread_create(&hiloNotify, NULL, verNotify, NULL);
	pthread_create(&hiloEjecuta, NULL, hiloEjecutador, NULL);

	borrarArchivos();

	inicializar();

	while(1){
		manejarSockets();
		verSiHayNuevosClientes();
	}
	return EXIT_SUCCESS;

}

void borrarArchivos(){
	remove("kernel.log");
}

void inicializar(){

	//Semaforos
	pthread_mutex_init(&mutex_config, NULL);
	pthread_mutex_init(&mutex_new,NULL);
	pthread_mutex_init(&mutex_ready,NULL);

	sem_init(&sem_new,0,0);
	sem_init(&sem_ready,0,0);
	sem_init(&sem_cpu,0,0);

	/*
	list_create(colaNew);
	list_create(colaReady);
	list_create(colaExec);
	list_create(colaExit);
*/
	//Crear Log
	logger = log_create("kernel.log","Kernel",0,LOG_LEVEL_INFO);

	//Configuracion
	cargarConfiguracion();
	inicializarSemaforos();
	inicializarVariablesCompartidas();
	mostrarConfiguracion();

	//Sockets

	//fileSystem = conectarConFileSystem();
	memoria = conectarConLaMemoria();
	prepararSocketsServidores();

	//Colas
	cola_new = queue_create();
	cola_exec = queue_create();
	cola_ready = queue_create();

	cola_block = queue_create();
	cola_exit = queue_create();
	cola_CPU_libres = queue_create();
}

void cargarConfiguracion() {
	pthread_mutex_lock(&mutex_config);

	printf("Cargando archivo de configuracion 'kernel.config'\n\n");
	log_info(logger, "Cargando archivo de configuracion 'kernel.config'");

	t_config* config = config_create(CONFIG_KERNEL);

	puerto_prog = config_get_int_value(config, "PUERTO_PROG");
	puerto_cpu = config_get_int_value(config, "PUERTO_CPU");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
	ip_fs = config_get_string_value(config, "IP_FS");
	puerto_fs = config_get_int_value(config, "PUERTO_FS");
	quantum = config_get_int_value(config, "QUANTUM");
	quantum_sleep = config_get_int_value(config, "QUANTUM_SLEEP");

	char* algor = config_get_string_value(config, "ALGORITMO");
	if(string_equals_ignore_case(algor, "FIFO")){
		algoritmo = 0;
	}
	else{
		algoritmo = 1;
	}
	free(algor);

	grado_multiprog = config_get_int_value(config, "GRADO_MULTIPROG");
	sem_ids = config_get_array_value(config, "SEM_IDS");
	sem_inits = config_get_array_value(config, "SEM_INIT");
	shared_vars = config_get_array_value(config, "SHARED_VARS");
	stack_size = config_get_int_value(config, "STACK_SIZE");

	log_info(logger, "Archivo de configuración cargado");
	pthread_mutex_unlock(&mutex_config);
}

void inicializarSemaforos(){

}

void inicializarVariablesCompartidas(){
	int i;
	int cantidadDeVariablesCompartidas = sizeof(shared_vars) / sizeof(shared_vars[0]);
	for(i = 0; i < cantidadDeVariablesCompartidas; i++){
		shared_vars[i] = 0;
	}
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
	printf("Algoritmo: %i \n", algoritmo);
	printf("Grado Multiprogramacion: %i \n", grado_multiprog);

	//Falta mostrar arrays

	printf("Stack Size: %i \n", stack_size);
}

void prepararSocketsServidores(){
	socketCPU = socket_escucha("127.0.0.1",puerto_cpu);
	listen(socketCPU,1);
	socketConsola = socket_escucha("127.0.0.1",puerto_prog);
	listen(socketConsola,1);

	log_info(logger, "Sockets escuchando");
}

void manejarSockets(){
	int i;
	//elimina todos los clientes que hayan cerrado conexion
	compactaClaves(socketCliente, &numeroClientes);

	//se inicia descriptor lectura
	FD_ZERO (&fds_activos);

	//se agrega para select el servidor
	FD_SET (socketCPU, &fds_activos);
	FD_SET (socketConsola, &fds_activos);

	//se agregan para el select los clientes ya conectados

	for ( i = 0; i < numeroClientes; ++i){
		FD_SET (socketCliente[i],&fds_activos);
	}

	//el valor del descriptor mas grande, si no hay, retorna 0

	socketMasGrande = dameSocketMasGrande(socketCliente, numeroClientes);

	if(socketMasGrande < socketConsola){
		socketMasGrande = socketConsola;
	}

	//esperamos hasta que haya un cambio
	select(socketMasGrande +1, &fds_activos, NULL, NULL, NULL);

	/* Se comprueba si algún cliente ya conectado ha enviado algo */
	for (i=0; i<numeroClientes; i++){
		t_paquete* paqueteRecibido;
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
		int res = nuevoClienteConsola(socketConsola, socketCliente, &numeroClientes);
		//Ver que pasa si no puede conectar
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

	bool resultado_CPU = esperar_handshake(clientes[*nClientes - 1], HandshakeCPUKernel);

	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
	if(resultado_CPU){
		log_info(logger, "Handshake OK, pedido de conexion cliente %d aceptado", *nClientes);
		cpusConectadas[indiceCPUsConectadas] = servidor;
		indiceCPUsConectadas++;
		cpusDisponibles[indiceCPUsDisponibles] = servidor;
		indiceCPUsDisponibles++;
		//intentarMandarProcesoACPU();
	}
	else{
		log_error(logger, "Handshake fallo, pedido de conexion rechazado");
		printf ("Handshake rechazado\n");
	}

	return;
}

int nuevoClienteConsola (int servidor, int *clientes, int *nClientes)
{
	int resultado;
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes] = aceptar_conexion(servidor);

	(*nClientes)++;

	/* Si se ha superado el maximo de clientes, se cierra la conexión,
	 * se deja todo como estaba y se vuelve. */

	if ((*nClientes) > MAX_CLIENTES) {
		close (clientes[(*nClientes) -1]);
		(*nClientes)--;
		resultado = 0 ;
	}

	bool resultado_Consola = esperar_handshake(clientes[*nClientes - 1], HandshakeConsolaKernel);
	/* Escribe en pantalla que ha aceptado al cliente y vuelve */

	//VER ESTO, que pasa si falla HANDSHAKE, que debo informar a Consola?
	if(resultado_Consola){
		//Cuando acepta el cliente, ya voy a crear la estructura del programa y le envío el PID
		log_info(logger, "Handshake OK, pedido de conexion cliente %d aceptado", *nClientes);
		resultado = 1;

	}
	else{
		log_error(logger, "Handshake fallo, pedido de conexion cliente %d rechazado", *nClientes);
		resultado = 0;
	}

	return resultado;
}



void procesarPaqueteRecibido(t_paquete* paqueteRecibido, un_socket socketActivo){
	switch(paqueteRecibido->codigo_operacion){
		case 101: //Codigo 101: Crear Script Ansisop
			nuevoProgramaAnsisop(socketActivo, paqueteRecibido);
			break;
		case 1000000: //Codigo a definir que indica fin de proceso en CPU y libero
			finalizarProcesoCPU(paqueteRecibido, socketActivo);
			break;
		default:
			break;
	}
}


void nuevoProgramaAnsisop(int* socket, t_paquete* paquete){
	int exito;
	t_proceso* proceso;
	t_proceso* procesoin;
	proceso = crearPrograma(socket);

	log_info(logger, "KERNEL: Creando proceso %d", proceso->pcb->pid);

	//ENVIO PID A CONSOLA
	enviar(proceso->socketConsola, EnvioPIDAConsola, sizeof(int), &proceso->pcb->pid);
	//envio cola NEW

	pthread_mutex_lock(&mutex_new);

	queue_push(cola_new,proceso);

	sem_post(&sem_new); // capaz que no es necesario, para que saque siempre 1, y no haga lio
	pthread_mutex_unlock(&mutex_new);

	if(cantidadDeProgramas > grado_multiprog){
		//No puedo pasar a Ready, aviso a Consola
		enviar(socket,108, sizeof(int), NULL );

		//Lo tendria que sacar de NEW para no hacer cagada
	}

	//Envio todos los datos a Memoria y espero respuesta

	exito = enviarCodigoAMemoria(paquete->data, paquete->tamanio, proceso, INICIALIZAR_PROCESO);

	if(exito == INICIALIZAR_PROCESO_OK){
	//Hay espacio asignado
		cantidadDeProgramas++; //sumo un pid mas en ejecucion

		//SACO DE NEW Y MANDO A READY
		sem_wait(&sem_new);
		pthread_mutex_lock(&mutex_new);
		procesoin = queue_pop(cola_new);
		pthread_mutex_unlock(&mutex_new);

		log_info(logger, "NUCLEO: saco proceso %d de NEW mando a READY", procesoin->pcb->pid);

		pthread_mutex_lock(&mutex_ready);
		queue_push(cola_ready,procesoin);
		pthread_mutex_unlock(&mutex_ready);

		sem_post(&sem_ready);
	}
	else{
		log_info(logger, "KERNEL: SIN ESPACIO EN MEMORIA, se cancela proceso");
		//ENVIO A CONSOLA ERROR POR MEMORIA
		enviar(socket, EnvioErrorAConsola, sizeof(int), NULL);

		//Se pasa de NEW directo a la cola EXIT con ExitCode -1
		sem_wait(&sem_new);
		pthread_mutex_lock(&mutex_new);
		procesoin = queue_pop(cola_new);
		pthread_mutex_unlock(&mutex_new);

		finalizarProceso(procesoin, FaltaDeMemoria);
	}

	liberar_paquete(paquete);
}


t_proceso* crearPrograma(int socketC){
	t_proceso* procesoNuevo;
	t_pcb * pcb;
	pcb = nalloc(sizeof(t_pcb));
	procesoNuevo = nalloc(sizeof(t_proceso));
	procesoNuevo->pcb = pcb;
	procesoNuevo->pcb->pid = pidcounter;
	procesoNuevo->socketConsola = socketC;
	pidcounter ++;
	return procesoNuevo;
}

/*
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

	//pasarProcesoALista(nuevoProcesoAnsisop->pcb, ListaNew, ListaNull);

	if(grado_multiprog < cantidadDeProgramas){
		//SI PERMITE, PASA DE NEW A READY
		//Pido paginas a memoria y memoria me dice si le alcanzan
		bool resultadoPedidoPaginas = pedirPaginasParaProceso(nuevoProcesoAnsisop);

		//Si la memoria devolvio OK, asigno la cantidad de paginas al PCB. Si no devolvió OK, lo finalizo con error por falta de memoria
		if(resultadoPedidoPaginas){
			pcb->pageCounter = resultadoPedidoPaginas;

			//pasarProcesoAlista(nuevoProcesoAnsisop->pcb, ListaReady, ListaNew);

			//intentarMandarProcesoACPU(nuevoProcesoAnsisop->pcb);
		}
		else {
			//FALTA ELIMINAR DE NEW
			finalizarProceso(nuevoProcesoAnsisop,-1);
		}
	}

}
*/
void finalizarProcesoCPU(t_paquete* paquete, un_socket socketCPU){
	//Desarmar paquete para ver codigo de finalizacion
	int pid = 0; //CAMBIAR POR PID DEL PAQUETE
	int codigoDeFinalizacion = 0; //CAMBIAR POR CODIGO DEL PAQUETE
	finalizarProceso(pid, codigoDeFinalizacion);
	//intentarMandarProcesoACPU();
}

void finalizarProceso(t_proceso* procesoAFinalizar, int exitCode){
	procesoAFinalizar->pcb->exitCode = exitCode;

	//AGREGAR PROCESO A COLA EXIT
	pthread_mutex_lock(&mutex_exit);
	queue_push(cola_exit, procesoAFinalizar);
	sem_post(&sem_exit);
	pthread_mutex_unlock(&mutex_exit);

	/*int codigoDeFinalizacion;
	if(exitCode == 0){
		//Codigo de Finzalicacion correcta del proceso
		codigoDeFinalizacion = 102;
	}
	else{
		//Codigo Kernel Aborto Proceso
		codigoDeFinalizacion = 106;
	}

	enviar(procesoAFinalizar->socketConsola, codigoDeFinalizacion, sizeof(int), NULL);*/
}

/*
bool pedirPaginasParaProceso(t_proceso* proceso){
	//Calculo paginas de memoria que necesito pedir de memoria para este script
	int paginasAPedir = ceil(paqueteRecibido->tamanio/TAMANIODEPAGINA);

	t_pedidoDePaginasKernel* pedidoDePaginas = malloc(sizeof(t_pedidoDePaginasKernel));
	pedidoDePaginas->pid = proceso->pcb->pid;
	pedidoDePaginas->paginasAPedir = paginasAPedir;

	//Envio a memoria el pedido de pagina
	enviar(memoria, 201, sizeof(t_pedidoDePaginasKernel),pedidoDePaginas);
	enviar(memoria, CodigoMemoriaKernel.ASIGNARPAGINAS, sizeof(int), paginasAPedir);

	free(pedidoDePaginas);

	//Tengo que esperar a que vuelva la respuesta del pedido. Si esta OK, devuelve la cantidad de paginas, sino devuelve -1
	t_paquete* respuestaAPedidoDePaginas;
	respuestaAPedidoDePaginas = recibir(memoria);

	if(respuestaAPedidoDePaginas->data == 0){
		return true;0
	}
	else{
		return false;
	}
}

*/
/*
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


*/
/*
void enviarUnProcesoACPU(un_socket socketCPU, int pid){
	//PASO EL PROCESO A EXEC Y LO MANDO A LA CPU CORRESPONDIENTE

	t_proceso* proceso = buscarProcesoEnReadySegunPID(pid);

	pasarProcesoALista(proceso, ListaExec, ListaReady);

	//51651 VER CODIGO DE PROCESO A EJECUTAR EN CPU
	enviar(socketCPU, 51651, sizeof(t_proceso), proceso);
}
*/
int conectarConLaMemoria(){
	t_paquete * paquete;
	pthread_mutex_lock(&mutex_config);
	log_info(logger, "MEMORIA: Inicio de conexion");
	un_socket socketMemoria = conectar_a(ip_memoria, puerto_memoria);

	if (socketMemoria == 0){
		log_error(logger, "MEMORIA: No se pudo conectar");
		pthread_mutex_unlock(&mutex_config);
		exit (EXIT_FAILURE);
	}

	log_info(logger, "MEMORIA: Recibio pedido de conexion de Kernel");

	log_info(logger, "MEMORIA: Iniciando Handshake");
	bool resultado = realizar_handshake(socketMemoria , HandshakeMemoriaKernel);
	if (resultado){
		log_info(logger, "MEMORIA: Handshake exitoso! Conexion establecida");
		//paquete = recibir(socketMemoria);
		//TAMPAG = *((int*)paquete->data);
		TAMPAG= 256;
		log_info(logger, "KERNEL: Tamano pagina de Memoria %d",TAMPAG);
		pthread_mutex_unlock(&mutex_config);
		return socketMemoria ;
	}
	else{
		log_error(logger, "MEMORIA: Fallo en el handshake, se aborta conexion");
		pthread_mutex_unlock(&mutex_config);
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
	bool resultado = realizar_handshake(socketFileSystem, HandshakeFileSystemKernel);
	if (resultado){
		log_info(logger, "FILESYSTEM: Handshake exitoso! Conexion establecida");
		return socketFileSystem;
	}
	else{
		log_error(logger, "FILESYSTEM: Fallo en el handshake, se aborta conexion");
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


void verNotify(){
	//printf("HILO NOTIFY\n");
	int file_descriptor = inotify_init();
	if(file_descriptor < 0) perror("inotify_init");
	int watch_descriptor = inotify_add_watch(file_descriptor, CONFIG_PATH, IN_MODIFY);
	char buffer[1000];


	while(1){
		int length = read(file_descriptor, buffer, 1000);
		int offset = 0;
		while (offset < length){
			struct inotify_event * event = (struct inotify_evento*) &buffer[offset];
			if (event->len){
				if(event->mask & IN_MODIFY){
					if(!(event->mask & IN_ISDIR)){
						if(strcmp(event->name, CONFIG_KERNEL_SOLO)==0){

							log_info(logger, "KERNEL: cambio el archivo config");
							usleep(500*1000);
							cargarConfiguracion();
							mostrarConfiguracion();
						}
					}
				}
			}
		offset += sizeof (struct inotify_event) + event->len;
		}
	}
}

void * nalloc(int tamanio){
	int i;
	void * retorno = malloc(tamanio);
	for(i=0;i<tamanio;i++) ((char*)retorno)[i]=0;
	return retorno;
}

int enviarCodigoAMemoria(char* codigo, int size, t_proceso* proceso, codigosMemoriaKernel codigoOperacion){
	int paginasAPedir = ceil((double)size / (double)TAMPAG);
	t_pedidoDePaginasKernel* pedidoDePaginas = malloc(sizeof(t_pedidoDePaginasKernel));
	pedidoDePaginas->pid = proceso->pcb->pid;
	pedidoDePaginas->paginasAPedir = paginasAPedir;

	//Envio a memoria el pedido de pagina
	enviar(memoria, codigoOperacion, sizeof(t_pedidoDePaginasKernel),pedidoDePaginas);

	free(pedidoDePaginas);

	//Tengo que esperar a que vuelva la respuesta del pedido. Si esta OK, devuelve la cantidad de paginas, sino devuelve -1
	t_paquete* respuestaAPedidoDePaginas;
	respuestaAPedidoDePaginas = recibir(memoria);

	return respuestaAPedidoDePaginas->codigo_operacion;
}

void hiloEjecutador(){
	//printf("hilo Ejecutador\n");
	t_proceso* proceso;
	int socketCPULibre;

	while(1){
		//printf("antes semaforo ready\n");
		sem_wait(&sem_ready);//El signal lo tengo cuando envio de New a Ready
		//sem_wait(&sem_cpu);//Cuando conecta CPU, sumo un signal y sumo una cpuLibre a la lista
		//printf("dentro while ejecuta\n");
		pthread_mutex_lock(&mutex_config); //Como envio despues datos para ejecutar, necesito mutex
		//socketCPULibre = (un_socket)queue_pop(cola_CPU_libres);
		pthread_mutex_lock(&mutex_ready);
		proceso = queue_pop(cola_ready);
		pthread_mutex_unlock(&mutex_ready);

		log_info(logger, "KERNEL: Saco proceso %d de Ready, se envia a Ejecutar", proceso->pcb->pid);

		mandarAEjecutar(proceso, socketCPULibre);


		//SIMULACION DE EJECUCION PARA PRUEBAS!!!

		printf("Estaria EJECUTANDO\n");
		sleep(4);
		printf("Termine de ejecutar\n");

		//Pruebo envio codigo 102 programa finalizado
		enviar(proceso->socketConsola, EnvioFinalizacionAConsola, sizeof(int), &proceso->pcb->pid);
		//dejar MUTEX
		pthread_mutex_unlock(&mutex_config);


	}
}

void mandarAEjecutar(t_proceso* proceso, int socket){
	//Serializar PCB

	proceso->socketCPU = socket;

	pthread_mutex_lock(&mutex_exec);
	queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&mutex_exec);

	//preparar datos de Kernel
	//enviar primero datos
	//enviar paquete serializado



}
