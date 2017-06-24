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

	//Crear Log
	logger = log_create("kernel.log","Kernel",true,LOG_LEVEL_TRACE);

	//Configuracion
	cargarConfiguracion();
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
	int j;
	printf("inicio\n");
	bool haySemaforos(){

		bool retorno = false;

		int i;

		for(i=0;i<strlen((char*)sem_inits)/sizeof(char*);i++){
			printf("i: %d\n");
			if(list_size(cola_semaforos[i]->elements)>0) retorno = true;
		}
		printf(4);
		return retorno;
	}

	if(hayConfiguracion){
		bool semaforo;
		int i;

		semaforo = haySemaforos();

		if(semaforo){
			log_info(logger, "KERNEL: No se puede cambiar la configuracion hasta que la cola de Semaforos este vacia");
			while(haySemaforos()){}
			log_info(logger, "KERNEL: Cola semaforos vacia, cambiando configuracion...");
		}
	}

	pthread_mutex_lock(&mutex_config);

	//printf("Cargando archivo de configuracion 'kernel.config'\n\n");

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

	valor_semaforos = convertirConfigEnInt(sem_inits);

	valor_shared_vars = iniciarSharedVars(shared_vars);

	if(cola_semaforos!= 0){
		free(cola_semaforos);
	}

	cola_semaforos = nalloc(strlen((char*)sem_inits)*sizeof(char*));
	for( j=0; j< strlen((char*)sem_inits) / sizeof(char*);j++){
		cola_semaforos[j] = nalloc(sizeof(t_queue*));
		cola_semaforos[j] = queue_create();
	}

	log_info(logger, "Archivo de configuración cargado");
	hayConfiguracion = true;
	pthread_mutex_unlock(&mutex_config);
}


int* convertirConfigEnInt(char** valores_iniciales){
	int i;
	int *resul;
	resul=nalloc(((strlen((char*)valores_iniciales))/sizeof(char*))* sizeof(int));
	for (i=0; i< (strlen((char*) valores_iniciales))/sizeof(char*);i++){
		resul[i] = atoi(valores_iniciales[i]);
	}
	return resul;
}


int* iniciarSharedVars(char** variables_compartidas){
	int i;
	int* resul;
	resul = nalloc(((strlen((char*)variables_compartidas))/sizeof(char*))*sizeof(int));
	for(i=0; i<(strlen((char*) variables_compartidas))/sizeof(char*);i++){
		resul[i]=0;
	}
	return resul;
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
			if(paqueteRecibido->codigo_operacion > -1){
				log_info(logger, "Me envio datos el socket %i", socketCliente[i]);
				 procesarPaqueteRecibido(paqueteRecibido, socketCliente[i]);
			}
			else {
				log_warning(logger, "El cliente %d se desconecto",i+1);
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
	un_socket clienteCPUtmp;
	int* infoAlgoritmo;
	/* Acepta la conexión con el cliente, guardándola en el array */
	clientes[*nClientes] = aceptar_conexion(servidor);
	clienteCPUtmp = clientes[*nClientes];
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
		infoAlgoritmo = algoritmo;

		enviar(clienteCPUtmp,ENVIAR_ALGORITMO,sizeof(int), &infoAlgoritmo);

		//Ponemos la CPU libre en la cola y hacemos Signal del semaforo CPU
		queue_push(cola_CPU_libres, (void*)clienteCPUtmp);
		sem_post(&sem_cpu);
		log_info(logger, "Nueva CPU agregada a lista de CPU_LIBRES");
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
		case ENVIAR_SCRIPT: //Codigo 101: Crear Script Ansisop
			nuevoProgramaAnsisop(socketActivo, paqueteRecibido);
			break;
		case PEDIR_SEMAFORO:
			pideSemaforo(socketActivo, paqueteRecibido);
			break;
		case 1000000: //Codigo a definir que indica fin de proceso en CPU y libero
			finalizarProcesoCPU(paqueteRecibido, socketActivo);
			break;
		default:
			break;
	}
}


int nuevoProgramaAnsisop(int* socket, t_paquete* paquete){
	int exito;
	t_proceso* proceso;
	t_proceso* procesoin;
	proceso = crearPrograma(socket);

	log_info(logger, "KERNEL: Creando proceso %d", proceso->pcb->pid);

	//ENVIO PID A CONSOLA
	enviar(proceso->socketConsola, ENVIAR_PID, sizeof(int), &proceso->pcb->pid);

	//envio cola NEW
	pthread_mutex_lock(&mutex_new);
	queue_push(cola_new, proceso);
	sem_post(&sem_new); // capaz que no es necesario, para que saque siempre 1, y no haga lio
	pthread_mutex_unlock(&mutex_new);

	printf("cantidad prog %d\n", cantidadDeProgramas);
	printf("grado multiprog%d\n", grado_multiprog);
	if(cantidadDeProgramas >= grado_multiprog){
		//No puedo pasar a Ready, aviso a Consola
		int* basura = 1;
		enviar(socket, 108, sizeof(int), &basura);
		//Lo saco de New, le asigno el exit code, y lo mando a Exit directo
		pthread_mutex_lock(&mutex_new);
		t_proceso* proceso = queue_pop(cola_new);
		sem_wait(&sem_new);
		pthread_mutex_unlock(&mutex_new);

		proceso->pcb->exitCode = -1;

		pthread_mutex_lock(&mutex_exit);
		queue_push(cola_exit, proceso);
		sem_post(&sem_exit);
		pthread_mutex_unlock(&mutex_exit);

		return -1;
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

		log_info(logger, "KERNEL: saco proceso %d de NEW mando a READY", procesoin->pcb->pid);

		pthread_mutex_lock(&mutex_ready);
		queue_push(cola_ready,procesoin);
		pthread_mutex_unlock(&mutex_ready);

		sem_post(&sem_ready);
		printf("pase todos semaforos bien \n");
	}
	else{
		log_error(logger, "KERNEL: SIN ESPACIO EN MEMORIA, se cancela proceso");
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
	return 0;
}


void pideSemaforo(int* socketActivo, t_paquete* paqueteRecibido){
	t_proceso* procesoPideSem;
	t_paquete* paqueteNuevo;
	t_pcb* pcb_temporal;
	pthread_mutex_lock(&mutex_exec);
	procesoPideSem = obtenerProcesoSocketCPU(cola_exec, socketActivo);
	pthread_mutex_unlock(&mutex_exec);
	pthread_mutex_lock(&mutex_config);

	log_info(logger, "Proceso %d pide semaforo %s", procesoPideSem->pcb->pid, paqueteRecibido->data);

	int* valorSemaforo = buscarSemaforo(paqueteRecibido->data);
	int mandar;
	if(*valorSemaforo<=0){
		mandar =1;
		log_info(logger, "KERNEL: Recibi proceso %d mando a bloquear por semaforo %s", procesoPideSem->pcb->pid, paqueteRecibido->data);

		enviar(socketActivo, PEDIDO_SEMAFORO_FALLO, sizeof(int), &mandar);//1 bloquea proceso
		paqueteNuevo = recibir(socketActivo);
		pcb_temporal = desserializarPCB(paqueteNuevo->data);
		liberar_paquete(paqueteNuevo);
		destruirPCB(procesoPideSem->pcb);
		procesoPideSem->pcb = pcb_temporal;

		/* VER
		if(procesoPideSem->abortado == false){
			bloqueoSemaforo(procesoPideSem, paqueteRecibido->data);
			if(flag==0){
				queue_push(cola_CPU_libres, (void*)socketActivo);
				sem_post(&sem_cpu);
			}
			else{
				log_info(logger,"KERNEL: se va a cerrar CPU");
			}
		}else{
			log_info(logger,"KERNEL: Se recibio proceso %d por fin de quantum, abortado", procesoPideSem->pcb->pid);
			abortar(procesoPideSem);
		}
		*/
	}
	else{
		escribeSemaforo(paqueteNuevo->data,*buscarSemaforo(paqueteRecibido)-1);
		mandar = 0;
		enviar(socketActivo, PEDIDO_SEMAFORO_OK, sizeof(int), &mandar);
		pthread_mutex_lock(&mutex_exec);
		queue_push(cola_exec, procesoPideSem);
		pthread_mutex_unlock(&mutex_exec);
	}

	pthread_mutex_unlock(&mutex_config);
}

t_pcb* desserializarPCB(char* serializado){
	t_pcb* pcb;
	return pcb;
}
void destruirPCB(t_pcb* pcb){

}

int* buscarSemaforo(char*semaforo){
	int i;

	for(i=0; i< strlen((char*)sem_ids)/sizeof(char*);i++){
		if(strcmp((char*)sem_ids[i], semaforo) ==0){
			return (&valor_semaforos[i]);
		}
	}
	log_error(logger, "No se encontro el semaforo %s", semaforo);
	exit(0);

}

void escribeSemaforo(char* semaforo, int valor){
	int i;

	for(i=0; i< strlen((char*)sem_ids)/sizeof(char*);i++){
		if(strcmp((char*)sem_ids[i], semaforo)==0){
			valor_semaforos[i] = valor;
			return;
		}
	}
	log_error(logger,"No se encontro semaforo %s", semaforo);
	exit(0);
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

void finalizarProcesoCPU(t_paquete* paquete, un_socket socketCPU){
	//Desarmar paquete para ver codigo de finalizacion
	int pid = 0; //CAMBIAR POR PID DEL PAQUETE
	int codigoDeFinalizacion = 0; //CAMBIAR POR CODIGO DEL PAQUETE
	finalizarProceso(pid, codigoDeFinalizacion);
}

void finalizarProceso(t_proceso* procesoAFinalizar, int exitCode){
	procesoAFinalizar->pcb->exitCode = exitCode;

	cantidadDeProgramas --;

	//AGREGAR PROCESO A COLA EXIT
	pthread_mutex_lock(&mutex_exit);
	queue_push(cola_exit, procesoAFinalizar);
	sem_post(&sem_exit);
	pthread_mutex_unlock(&mutex_exit);


	/*
	 *
	 *                 HACER QUE LOS CODIGOS DE FINALIZACION QUE RECONOCE CONSOLA SEAN LOS MISMOS, AL PEDO OTROS
	 *
	 * */
	int* basura = 0;
	enviar(procesoAFinalizar->socketConsola, procesoAFinalizar->pcb->exitCode, sizeof(int), &basura);
}


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
		log_debug(logger, "MEMORIA: Handshake exitoso! Conexion establecida");
		paquete = recibir(socketMemoria);
		if(paquete->codigo_operacion == TAMANIO_PAGINA){
			TAMPAG = *((int*)paquete->data);
			//TAMPAG= 256;
			log_info(logger, "KERNEL: Tamano pagina de Memoria %d",TAMPAG);
		}
		else{
			//QUE PASA EN ESTE CASO?
			log_error(logger, "KERNEL: Error en el envio del TAMPAG");
		}
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
		log_error(logger, "No se pudo conectar con FileSystem");
		exit (EXIT_FAILURE);
	}

	log_info(logger, "FILESYSTEM: Recibio pedido de conexion de Kernel");

	log_info(logger, "FILESYSTEM: Iniciando Handshake");
	bool resultado = realizar_handshake(socketFileSystem, HandshakeFileSystemKernel);
	if (resultado){
		log_debug(logger, "FILESYSTEM: Handshake exitoso! Conexion establecida");
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
		sem_wait(&sem_ready);//El signal lo tengo cuando envio de New a Ready
		//sem_wait(&sem_cpu);//Cuando conecta CPU, sumo un signal y sumo una cpuLibre a la lista
		pthread_mutex_lock(&mutex_config); //Como envio despues datos para ejecutar, necesito mutex
		//socketCPULibre = (un_socket)queue_pop(cola_CPU_libres);
		pthread_mutex_lock(&mutex_ready);
		proceso = queue_pop(cola_ready);
		pthread_mutex_unlock(&mutex_ready);

		log_info(logger, "KERNEL: Saco proceso %d de Ready, se envia a Ejecutar", proceso->pcb->pid);

		//mandarAEjecutar(proceso, socketCPULibre);


		/*SIMULACION DE EJECUCION PARA PRUEBAS!!!*/

		printf("Estaria EJECUTANDO\n");
		sleep(20);
		printf("Termine de ejecutar\n");

		//Pruebo envio codigo 102 programa finalizado
		enviar(proceso->socketConsola, EnvioFinalizacionAConsola, sizeof(int), &proceso->pcb->pid);
		finalizarProceso(proceso, 0);

		/*FIN SIMULACION DE EJECUCION PARA PRUEBAS*/


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


t_proceso* obtenerProcesoSocketCPU(t_queue *cola, int socketBuscado){
	int a = 0, t;
	t_proceso*proceso;
	while(proceso = (t_proceso*)list_get(cola->elements, a)){
		if (proceso->socketCPU == socketBuscado) return (t_proceso*)list_remove(cola->elements, a);
		a++;
	}
	log_error(logger, "No hay proceso para retirar");
	exit(0);
	return NULL;
}
