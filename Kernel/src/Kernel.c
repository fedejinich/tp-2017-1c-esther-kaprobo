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
	pthread_create(&hiloPCP, NULL, planificadorCortoPlazo, NULL);
	pthread_create(&hiloConsolaKernel, NULL, hiloConKer, NULL);

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
	pthread_mutex_init(&mutex_new, NULL);
	pthread_mutex_init(&mutex_ready, NULL);
	pthread_mutex_init(&mutexEjecuta, NULL);
	pthread_mutex_init(&mutex_listaHeap, NULL);

	sem_init(&sem_new, 0, 0);
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_cpu, 0, 0);

	//Crear Log
	logger = log_create("kernel.log","Kernel",true,LOG_LEVEL_TRACE);

	//Configuracion
	cargarConfiguracion();
	mostrarConfiguracion();

	//Sockets
	fileSystem = conectarConFileSystem();
	memoria = conectarConLaMemoria();
	prepararSocketsServidores();

	//Colas
	cola_new = queue_create();
	cola_exec = queue_create();
	cola_ready = queue_create();

	cola_block = queue_create();
	cola_exit = queue_create();
	cola_CPU_libres = queue_create();

	listaAdminHeap = list_create();
	tablaGlobalDeArchivos = list_create();
	tablaDeArchivosPorProceso = list_create();;
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
		t_datos_kernel datos_kernel;

		datos_kernel.QUANTUM= quantum;
		datos_kernel.QUANTUM_SLEEP = quantum_sleep;
		datos_kernel.TAMANIO_PAG = TAMPAG;
		datos_kernel.STACK_SIZE = stack_size;

		enviar(clienteCPUtmp, DATOS_KERNEL, sizeof(t_datos_kernel), &datos_kernel);

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

		case ENVIAR_SCRIPT: //Consola nos envia Script
			log_info(logger,"KERNEL: Se recibio nuevo Script desde Consola");
			nuevoProgramaAnsisop(socketActivo, paqueteRecibido);
			break;

		case PEDIR_SEMAFORO: //Proceso CPU nos pide Semaforo
			log_info(logger,"KERNEL: Proceso nos solicita semaforo");
			pideSemaforo(socketActivo, paqueteRecibido);
			break;

		case LIBERAR_SEMAFORO: //Proceso CPU nos libera semaforo
			log_info(logger,"KERNEL: Proceso nos libera semaforo");
			liberarSemaforo(socketActivo, paqueteRecibido);
			break;

		case ESCRIBIR_ARCHIVO: //Proceso CPU nos pide escribir texto, a traves de print. Se envia tb a Consola
			log_info(logger,"KERNEL: Proceso nos solicita imprimir texto");
			imprimirConsola(socketActivo, paqueteRecibido);
			//VER - Si el FD es 1 se imprime por consola CAMBIAR ESTO
			break;

		case ABRIR_ARCHIVO: //Proceso CPU nos pide abrir un archivo para un proceso dado
			abrirArchivo(socketActivo, paqueteRecibido);
			break;

		case OBTENER_DATOS: //Proceso CPU nos pide leer un archivo para un proceso dado
			accederArchivo(socketActivo, paqueteRecibido, 'r');
			break;

		case GUARDAR_DATOS: //Proceso CPU nos pide escribir un archivo para un proceso dado
			accederArchivo(socketActivo, paqueteRecibido, 'w');
			break;

		case CERRAR_ARCHIVO: //Proceso CPU nos pide cerrar un archivo para un proceso dado
			cerrarArchivo(socketActivo, paqueteRecibido);
			break;

		case SOLICITAR_VARIABLE: //Proceso CPU nos pide variable compartida
			log_info(logger, "KERNEL: Proceso nos solicita variable compartida");
			solicitaVariable(socketActivo, paqueteRecibido);
			break;

		case SOLICITAR_HEAP:
			log_info(logger,"KERNEL: Proceso nos solicita espacio Dinamico");

			reservarHeap(socketActivo, paqueteRecibido);
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
	t_proceso* proceso = malloc(sizeof(t_proceso));
	t_proceso* procesoin = malloc(sizeof(t_proceso));
	proceso = crearPrograma(socket, paquete);

	log_info(logger, "KERNEL: Creando proceso %d", proceso->pcb->pid);


	enviar(proceso->socketConsola, ENVIAR_PID, sizeof(int), &proceso->pcb->pid);


	pthread_mutex_lock(&mutex_new);
	queue_push(cola_new, proceso);
	sem_post(&sem_new); // capaz que no es necesario, para que saque siempre 1, y no haga lio
	pthread_mutex_unlock(&mutex_new);
	pthread_mutex_lock(&mutexGradoMultiprogramacion);
	if(cantidadDeProgramas >= grado_multiprog){
		pthread_mutex_unlock(&mutexGradoMultiprogramacion);
		int basura = 999;
		enviar((un_socket)socket, ERROR_MULTIPROGRAMACION, sizeof(int), (void*)basura);

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
	pthread_mutex_unlock(&mutexGradoMultiprogramacion);

	char* codigo = paquete->data;
	exito = inicializarProcesoYAlmacenarEnMemoria(codigo, paquete->tamanio, proceso);

	if(exito == EXIT_SUCCESS_CUSTOM){

		cantidadDeProgramas++; //sumo un pid mas en ejecucion

		//SACO DE NEW Y MANDO A READY
		sem_wait(&sem_new);
		pthread_mutex_lock(&mutex_new);
		procesoin = queue_pop(cola_new);
		pthread_mutex_unlock(&mutex_new);

		log_debug(logger, "KERNEL: saco proceso %d de NEW mando a READY", procesoin->pcb->pid);

		pthread_mutex_lock(&mutex_ready);
		queue_push(cola_ready,procesoin);
		pthread_mutex_unlock(&mutex_ready);

		sem_post(&sem_ready);

	}
	else {
		//NO SIEMPRE VA A SER POR FALTA DE MEMORIA
		//PUEDE FALLAR PORQUE SE QUIERE ALMACENAR CODIGO DE UN PID INEXISTENTE EN TABLA DE PAGINAS

		log_error(logger, "KERNEL: SIN ESPACIO EN MEMORIA, se cancela proceso");

		enviar((un_socket)socket, EnvioErrorAConsola, sizeof(int), NULL);

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
	t_proceso* procesoPideSem = malloc(sizeof(t_proceso));
	t_paquete* paqueteNuevo = malloc(sizeof(t_paquete));
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

		enviar((un_socket)socketActivo, PEDIDO_SEMAFORO_FALLO, sizeof(int), &mandar);//1 bloquea proceso
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
		enviar((un_socket)socketActivo, PEDIDO_SEMAFORO_OK, sizeof(int), &mandar);
		pthread_mutex_lock(&mutex_exec);
		queue_push(cola_exec, procesoPideSem);
		pthread_mutex_unlock(&mutex_exec);
	}

	pthread_mutex_unlock(&mutex_config);
}

void liberarSemaforo(int* socketActivo, t_paquete* paqueteRecibido){

}

void imprimirConsola(int* socketActivo, t_paquete* paqueteRecibido){
	// ver ME llega en el paquete el FD, la info y el tamanio, hay que definir como hacerlo bien
	t_proceso* proceso;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec, (un_socket)socketActivo);
	queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&mutex_exec);
	if(proceso->abortado==false)
		enviar((un_socket)(proceso->socketConsola), IMPRIMIR_CONSOLA, paqueteRecibido->tamanio, paqueteRecibido->data);
	return;
}

void abrirArchivo(int* socketActivo, t_paquete* paquete){
	//en el paquete esta el path del archivo y los permisos de apertura
	t_envioDeDatosKernelFSAbrir* datosProcesoArchivo = paquete->data;
	int pid = datosProcesoArchivo->pid;
	char* path = datosProcesoArchivo->path;
	char* permisos = datosProcesoArchivo->permisos;

	if(validarPermisoDeApertura(pid, path, permisos)){
		//se manda al fs el path
		enviar(fileSystem, SOLICITUD_APERTURA_ARCHIVO, sizeof(path), path);

		t_paquete* paqueteResultado = recibir(memoria);

		int resultado = paqueteResultado->codigo_operacion;

		//si esta ok, se genera un nuevo FD para el archivo y se lo ingresa en la tabla de archivos del proceso
		if(resultado == ARCHIVO_ABIERTO){
			t_tablaDeArchivosDeUnProceso* entradaLocal = malloc(sizeof(t_tablaDeArchivosDeUnProceso));

			//Se fija si esta en la tabla global, si esta, agarra el fd
			entradaLocal->flags = permisos;
			entradaLocal->globalFD = chequearTablaGlobal(path);

			//Agrego la entrada a la tabla en el indice = pid, que es la tabla correspondiente
			list_add_in_index(tablaDeArchivosPorProceso, pid, entradaLocal);
		}
		else{
			finalizarProcesoPorPID(pid, ErrorSinDefinicion);
			enviar(socketActivo, ARCHIVO_NO_SE_PUDO_ABRIR, sizeof(resultado), resultado);
			return;
		}

		//se avisa a cpu si se pudo o no abrir el archivo
		//enviar(fileSystem, resultado, sizeof(resultado), resultado);
	}
}

bool validarPermisoDeApertura(int pid, char* path, char* permisos){
	bool permisoParaSeguir = false;
	if(strchr(&permisos, 'c') != NULL){
		permisoParaSeguir = true;
	}
	else{
		if(existeArchivo(path)){
			permisoParaSeguir = true;
		}
		else{
			finalizarProceso(pid, ArchivoInexistente);
		}
	}
	return permisoParaSeguir;
}

bool existeArchivo(char* path){
	bool resultado = false;

	enviar(fileSystem, VALIDAR_ARCHIVO, sizeof(path), path);

	t_paquete* paqueteResultado = recibir(fileSystem);

	if(paqueteResultado->codigo_operacion == ARCHIVO_EXISTE)
		resultado = true;

	return resultado;
}

int chequearTablaGlobal(char* path){
	int fd;
	int indiceEntrada = buscarEntradaEnTablaGlobal(path);
	if(indiceEntrada != -1){
		fd = indiceEntrada;
	}
	else{
		t_entradaTablaGlobalArchivos* entrada = malloc(sizeof(t_entradaTablaGlobalArchivos));
		entrada->path = path;
		entrada->open = 1;
		list_add(tablaGlobalDeArchivos, entrada);
		fd = buscarEntradaEnTablaGlobal(path);
	}
	return fd;
}

int buscarEntradaEnTablaGlobal(char* path){
	int a;
	t_entradaTablaGlobalArchivos* entrada;
	while(entrada = (t_entradaTablaGlobalArchivos*)list_get(tablaGlobalDeArchivos, a)){
		if (entrada->path == path){
			entrada->open++;
			return a;
		}
		a++;
	}
	return -1;
}

void accederArchivo(int* socketActivo, t_paquete* paquete, char operacion){
	t_envioDeDatosKernelFSLecturaYEscritura* datos= paquete->data;
	int pid = datos->pid;
	int fd = datos->fd;

	t_tablaDeArchivosDeUnProceso* tablaDeUnProceso = malloc(sizeof(t_tablaDeArchivosDeUnProceso));

	tablaDeUnProceso = list_get(tablaDeArchivosPorProceso, pid);
	t_tablaDeArchivosDeUnProceso* entrada = list_get(tablaDeUnProceso, fd);

	char* permisos = entrada->flags;

	int codigoOperacion;
	int exitCode;
	if(strchr(operacion, 'r') ){
		codigoOperacion = SOLICITUD_OBTENCION_DATOS;
		exitCode = IntentoDeLecturaSinPermisos;
	}
	else{
		codigoOperacion = SOLICITUD_GUARDADO_DATOS;
		exitCode = IntentoDeEscrituraSinPermisos;
	}
	if(strchr(permisos, 'r') != NULL){
		enviar(fileSystem, codigoOperacion, sizeof(datos), datos);
		char* buffer = recibir(fileSystem);
		enviar(socketActivo, OBTENER_DATOS, datos->tamanio, buffer);
	}
	else{
		finalizarProceso(pid, exitCode);
	}
	//hay que traducir ese FD a un path
	//realizar peticion de lectura al fs con valores de path, offset y tamaño de lo que se desea leer
	//recibir lo que se queria leer
	//luego, enviar a cpu lo leido, o informar de error
}

void cerrarArchivo(int* socketActivo, t_paquete* paquete){
	t_envioDeDatosKernelFSLecturaYEscritura* datos= paquete->data;
	int pid = datos->pid;
	int fd = datos->fd;

	t_tablaDeArchivosDeUnProceso* entradaTablaProceso = obtenerEntradaTablaArchivosDelProceso(pid, fd);
	t_entradaTablaGlobalArchivos* entradaTablaGlobal = obtenerEntradaTablaGlobalDeArchivos(entradaTablaProceso);

	if(entradaTablaGlobal->open > 1){
		//Borrar la entrada de la tabla del proceso
		borrarArchivoDeTabla(pid, fd);

		//Actualizo el open
		entradaTablaGlobal->open--;
	}
	else{
		//Borrar la entrada de la tabla del proceso
		borrarArchivoDeTabla(pid, fd);
		log_warning(logger, "Archivo %i eliminado de la tabla del proceso %i", fd, pid);

		//Borrar la entrada de la tabla global
		list_remove(tablaGlobalDeArchivos, entradaTablaProceso->globalFD);
		log_warning(logger, "Archivo %i eliminado de la tabla de archivos globales", entradaTablaProceso->globalFD);

		//Avisar a FS que cierre el archivo
		enviar(fileSystem, CERRAR_ARCHIVO_FS, sizeof(entradaTablaGlobal->path), entradaTablaGlobal->path);
	}
}

t_tablaDeArchivosDeUnProceso* obtenerEntradaTablaArchivosDelProceso(int pid, int fd){
	t_tablaDeArchivosDeUnProceso* tablaDeUnProceso = malloc(sizeof(t_tablaDeArchivosDeUnProceso));
	tablaDeUnProceso = list_get(tablaDeArchivosPorProceso, pid);
	t_tablaDeArchivosDeUnProceso* entradaTablaDelProceso = list_get(tablaDeUnProceso, fd);

	return entradaTablaDelProceso;
}

t_entradaTablaGlobalArchivos* obtenerEntradaTablaGlobalDeArchivos(t_tablaDeArchivosDeUnProceso* entradaTablaDelProceso){
	return list_get(tablaGlobalDeArchivos, entradaTablaDelProceso->globalFD);
}

void borrarArchivoDeTabla(int pid, int fd){
	t_tablaDeArchivosDeUnProceso* tablaDeUnProceso = malloc(sizeof(t_tablaDeArchivosDeUnProceso));
	tablaDeUnProceso = list_get(tablaDeArchivosPorProceso, pid);
	list_remove(tablaDeUnProceso, fd);
}


void solicitaVariable(int* socketActivo, t_paquete* paqueteRecibido){
	int* valor;
	t_proceso* proceso;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec,socketActivo);
	queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&mutex_exec);
	pthread_mutex_lock(&mutex_config);
	valor = valorVariable(paqueteRecibido->data);
	enviar((un_socket)socketActivo,SOLICITAR_VARIABLE_OK, sizeof(int), &valor);
	pthread_mutex_unlock(&mutex_config);
	return;
}

int* valorVariable(char* variable){
	int i;
	log_info(logger, "Se solicita variable %s", variable);
	for(i=0; i < strlen((char*)shared_vars)/ sizeof(char*); i++){
		if(strcmp((char*)shared_vars[i], variable)==0){
			return &valor_shared_vars[i];
		}
	}
	log_error(logger, "KERNEL, no se encontro variable %s, exit", variable);
	exit(0);
}

void escribirVariable(int* socketActivo, t_paquete* paqueteRecibido){
	char* variable;
	t_proceso* proceso;
	int* valor;
	int i;

	pthread_mutex_lock(&cola_exec);
	proceso=obtenerProcesoSocketCPU(cola_exec, socketActivo);
	queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&cola_exec);
	pthread_mutex_lock(&mutex_config);

	//Para escribir primero enviamos el int y despues el string con el nombre
	variable = paqueteRecibido->data;
	valor = (int*)variable;
	variable +=4;
	for(i=0; i<strlen((char*)shared_vars)/sizeof(char*);i++){
		if(strcmp((char*)shared_vars[i], variable)==0){
			memcpy(&valor_shared_vars[i], valor, sizeof(int));
			return;
		}
	}
	log_error(logger, "No se encontro Variable %s, EXIT",variable);
	exit(0);
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

t_proceso* crearPrograma(int socketC , t_paquete* paquete){
	int size = paquete->tamanio;
	char* codigo = paquete->data;
	//printf("codigo: \n %s \n", codigo);
	t_proceso* procesoNuevo;
	t_pcb * pcb;
	int   i;

	t_metadata_program* metadata;

	pcb = nalloc(sizeof(t_pcb));
	procesoNuevo = nalloc(sizeof(t_proceso));
	procesoNuevo->pcb = pcb;
	procesoNuevo->pcb->pid = pidcounter;
	procesoNuevo->socketConsola = socketC;
	procesoNuevo->abortado = false;
	procesoNuevo->sizePaginasHeap = 0;
	pidcounter ++;

	metadata = metadata_desde_literal(codigo);
	printf("intrucciones: %d \n", metadata->instrucciones_size);

	procesoNuevo->pcb->paginasDeCodigo = ceil((double)size / (double)TAMPAG);
	procesoNuevo->pcb->sizeIndiceDeCodigo = metadata->instrucciones_size;
	procesoNuevo->pcb->indiceDeCodigo = malloc(procesoNuevo->pcb->sizeIndiceDeCodigo * 2 * sizeof(int));

	//Indice del Codigo
	for(i=0; i<metadata->instrucciones_size; i++){
		log_info(logger, "Instruccion inicio:%d offset:%d %.*s", metadata->instrucciones_serializado[i].start, metadata->instrucciones_serializado[i].offset, metadata->instrucciones_serializado[i].offset, codigo+metadata->instrucciones_serializado[i].start);
		procesoNuevo->pcb->indiceDeCodigo[i*2] = metadata->instrucciones_serializado[i].start;
		procesoNuevo->pcb->indiceDeCodigo[i*2 + 1]= metadata->instrucciones_serializado[i].offset;
	}

	procesoNuevo->pcb->sizeIndiceEtiquetas = metadata->etiquetas_size;
	procesoNuevo->pcb->indiceEtiquetas = malloc(procesoNuevo->pcb->sizeIndiceEtiquetas * sizeof(char));
	memcpy(procesoNuevo->pcb->indiceEtiquetas, metadata->etiquetas, procesoNuevo->pcb->sizeIndiceEtiquetas*sizeof(char));

	procesoNuevo->pcb->contextoActual = list_create();

	t_contexto* contextoInicial;
	contextoInicial = malloc(sizeof(t_contexto));

	contextoInicial->args = list_create();
	contextoInicial->vars = list_create();

	contextoInicial->sizeVars = 0;
	contextoInicial->sizeArgs = 0;
	contextoInicial->pos = 0;

	list_add(procesoNuevo->pcb->contextoActual, (void*)contextoInicial);

	procesoNuevo->pcb->sizeContextoActual = 1;
	procesoNuevo->pcb->programCounter = 0;
	(procesoNuevo->pcb->paginasDeMemoria)= (int)ceil((double)stack_size);

	metadata_destruir(metadata);


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

	if (socketMemoria < 0){
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
	un_socket socketFileSystem = conectar_a(ip_fs, (char*)puerto_fs);

	if (socketFileSystem < 0){
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

int inicializarProcesoYAlmacenarEnMemoria(char* codigo, int tamanioCodigo, t_proceso* proceso){
	log_info(logger, "Inicializando proceso. PID %i", proceso->pcb->pid);
	t_inicializar_proceso* paqueteProceso = malloc(sizeof(t_inicializar_proceso));

	int paginasTotales = proceso->pcb->paginasDeCodigo + proceso->pcb->paginasDeMemoria;
	log_info(logger, "Cantidad de paginas totales de PID %i: %i", proceso->pcb->pid, paginasTotales);
	log_info(logger, "Cantidad de paginas de codigo de PID %i: %i", proceso->pcb->pid, proceso->pcb->paginasDeCodigo);
	log_info(logger, "Cantidad de paginas de stack de PID %i: %i", proceso->pcb->pid, proceso->pcb->paginasDeMemoria);

	paqueteProceso->codigo = codigo;
	paqueteProceso->paginasTotales = paginasTotales;
	paqueteProceso->paginasCodigo = proceso->pcb->paginasDeCodigo;
	paqueteProceso->paginasStack = proceso->pcb->paginasDeMemoria;
	paqueteProceso->sizeCodigo = tamanioCodigo;
	paqueteProceso->pid = proceso->pcb->pid;

	enviar(memoria, INICIALIZAR_PROCESO, sizeof(t_inicializar_proceso), paqueteProceso);

	t_paquete * paquete = recibir(memoria);

	if(paquete->codigo_operacion == INICIALIZAR_PROCESO_FALLO) {
		log_error(logger, "No se pudo inicializar proceso");
		return EXIT_FAILURE_CUSTOM;
	}

	log_debug(logger, "Proceso inicializado. PID %i", proceso->pcb->pid);

	liberar_paquete(paquete);

	int exito = almacenarCodigoEnMemoria(proceso->pcb->pid, proceso->pcb->paginasDeCodigo, codigo);

	if(exito == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "No se pudo almacenar en memoria el codigo");
		return EXIT_FAILURE_CUSTOM;
	}

	return exito;
}

int almacenarCodigoEnMemoria(int pid, int paginasCodigo, char* codigo) {
	log_info(logger, "Almacenando codigo en memoria. PID %i, Paginas codigo %i, Tamanio codigo", pid, paginasCodigo, codigo, strlen(codigo));

	t_list* codigosParciales = getCodigosParciales(codigo, TAMPAG);

	int i;
	for(i = 1; i <= paginasCodigo; i++) {
		char* codigoParcial = list_get(codigosParciales, i-1);
		int tamanioCodigoParcial = strlen(codigoParcial);
		int pagina = i;
		int offset = 0;

		almacenarEnMemoria(memoria, logger, pid, pagina, offset, tamanioCodigoParcial, codigoParcial);
	}

	list_destroy(codigosParciales);

	log_debug(logger, "Codigo de PID %i almacenado en memoria", pid);
	return EXIT_SUCCESS_CUSTOM;
}

t_list* getCodigosParciales(char* codigo, int size) {
	t_list* codigosParciales = list_create();
	int i;
	int tamanioCodigo = strlen(codigo);
	for(i = 0; i < tamanioCodigo; i = i + size) {
		char* codigoParcial = string_substring(codigo, i, size);
		list_add(codigosParciales,codigoParcial);

	}

	return codigosParciales;
}

void planificadorCortoPlazo(){

	t_proceso* proceso;
	int socketCPULibre;

	while(1){
		if(!yaMeFijeReady){
			sem_wait(&sem_ready);//El signal lo tengo cuando envio de New a Ready
			yaMeFijeReady = true;
		}
		if(estadoPlanificacion){
			yaMeFijeReady = false;
			sem_wait(&sem_cpu);//Cuando conecta CPU, sumo un signal y sumo una cpuLibre a la lista
			pthread_mutex_lock(&mutex_config);
			socketCPULibre = (un_socket)queue_pop(cola_CPU_libres);

			pthread_mutex_lock(&mutex_ready);
			proceso = queue_pop(cola_ready);
			pthread_mutex_unlock(&mutex_ready);

			log_info(logger, "KERNEL: Saco proceso %d de Ready, se envia a Ejecutar", proceso->pcb->pid);

			mandarAEjecutar(proceso, socketCPULibre);

			pthread_mutex_unlock(&mutex_config);
		}
		else{
			printf("PLANIF DETENIDA \n");
			sleep(2);
		}
	}
}

void mandarAEjecutar(t_proceso* proceso, int socket){

	t_pcb* pcbSerializado;

	pcbSerializado = (t_pcb*)serializarPCB(proceso->pcb);

	proceso->socketCPU = socket;

	pthread_mutex_lock(&mutex_exec);
	queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&mutex_exec);

	t_datos_kernel datos_kernel;

	datos_kernel.QUANTUM= quantum;
	datos_kernel.QUANTUM_SLEEP = quantum_sleep;
	datos_kernel.TAMANIO_PAG = TAMPAG;
	datos_kernel.STACK_SIZE = stack_size;

	enviar(socket, DATOS_KERNEL, sizeof(t_datos_kernel), &datos_kernel);

	enviar(socket, PCB_SERIALIZADO, pcbSerializado->sizeTotal, (char*)pcbSerializado);

	/*SIMULACION DE EJECUCION PARA PRUEBAS SECCION 1*/

	printf("Estaria EJECUTANDO\n");
	sleep(10);
	printf("Termine de ejecutar\n");

	/*FIN SIMULACION DE EJECUCION PARA PRUEBAS SECCION 1*/


	pthread_mutex_lock(&mutex_exec);
	queue_pop(cola_exec);
	pthread_mutex_unlock(&mutex_exec);

	/*SIMULACION DE EJECUCION PARA PRUEBAS SECCION 2*/

	//Pruebo envio codigo 102 programa finalizado
	enviar(proceso->socketConsola, FINALIZAR_PROGRAMA, sizeof(int), &proceso->pcb->pid);
	finalizarProceso(proceso, 0);

	/*FIN SIMULACION DE EJECUCION PARA PRUEBAS SECCION 2*/
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

void hiloConKer(){
	while(1){
		pthread_mutex_lock(&mutexEjecuta);
		mostrarMenu();
		pthread_mutex_unlock(&mutexEjecuta);
		scanf("%i",&opcion);

		//opciones para consola
		switch(opcion) {
		case 1 :
			//Obtiene listado de procesos, todos o la cola seleccionada
			pthread_mutex_lock(&mutexEjecuta);
			mostrarListadoDeProcesos();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 2:
			//Obtiene la informacion de un proceso en particular
			pthread_mutex_lock(&mutexEjecuta);
			printf("Ingrese PID del proceso deseado\n");
			scanf("%i",&opcionPID);
			mostrarInformacionDeProceso(opcionPID);
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 3:
			//Obtiene la tabla global de archivos
			pthread_mutex_lock(&mutexEjecuta);
			mostrarTablaGlobalDeArchivos();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 4:
			//Modifica el grado de multiprogramacion del sistema
			pthread_mutex_lock(&mutexEjecuta);
			printf("Ingrese el grado de multiprogramacion deseado\n");
			scanf("%i",&opcionMultiProg);
			cambiarGradoMultiprogramacion(opcionMultiProg);
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 5:
			//Finaliza un proceso
			pthread_mutex_lock(&mutexEjecuta);
			printf("Ingrese PID del proceso a finalizar\n");
			scanf("%i",&opcionPID);
			finalizarProcesoPorPID(opcionPID, FinalizacionPorConsolaDeKernel);
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 6:
			//Detiene la planificacion
			pthread_mutex_lock(&mutexEjecuta);
			if(estadoPlanificacion){
				//sem_wait(sem_planificacion);
				estadoPlanificacion = false;
				log_warning(logger, "PLANIFICACION DETENIDA");
			}else{
				//sem_post(sem_planificacion);
				estadoPlanificacion = true;
				log_warning(logger, "PLANIFICACION CONTINUADA");
			}
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		default:
			printf("Opcion invalida, vuelva a intentar\n\n");
			break;
		}

	}
}

void mostrarMenu(){
	printf("\nIngrese la opcion deseada:\n");
	printf("OPCION 1 - OBTENER LISTADO DE PROCESOS\n");
	printf("OPCION 2 - OBTENER INFORMACION DE PROCESO\n");
	printf("OPCION 3 - OBTENER TABLA GLOBAL DE ARCHIVOS\n");
	printf("OPCION 4 - MODIFICAR GRADO DE MULTIPROGRAMACION\n");
	printf("OPCION 5 - FINALIZAR UN PROCESO\n");
	printf("OPCION 6 - DETENER/CONTINUAR LA PLANIFICACION\n");
	printf("Su Opcion:\n");
}

void mostrarListadoDeProcesos(){
	printf("Cola New\n");
	pthread_mutex_lock(&mutex_new);
	mostrarUnaListaDeProcesos(cola_new);
	pthread_mutex_unlock(&mutex_new);

	printf("Cola Ready\n");
	pthread_mutex_lock(&mutex_ready);
	mostrarUnaListaDeProcesos(cola_ready);
	pthread_mutex_unlock(&mutex_ready);

	printf("Cola Exec\n");
	pthread_mutex_lock(&mutex_exec);
	mostrarUnaListaDeProcesos(cola_exec);
	pthread_mutex_unlock(&mutex_exec);

	printf("Cola Block\n");
	pthread_mutex_lock(&mutex_block);
	mostrarUnaListaDeProcesos(cola_block);
	pthread_mutex_unlock(&mutex_block);

	printf("Cola Exit\n");
	pthread_mutex_lock(&mutex_exit);
	mostrarUnaListaDeProcesos(cola_exit);
	pthread_mutex_unlock(&mutex_exit);
}

void mostrarUnaListaDeProcesos(t_queue* colaAMostrar){
	int i = 0;
	t_proceso*proceso;
	while(proceso = (t_proceso*)list_get(colaAMostrar->elements, i)){
		printf("PID: %d\n", proceso->pcb->pid);
		i++;
	}
	if(i == 0){
		printf("No hay procesos en esta cola\n");
	}
	printf("\n");
}

void mostrarInformacionDeProceso(int pid){
	t_queue* colaDelProceso = buscarProcesoEnLasColas(pid);
	t_proceso* proceso = obtenerProcesoPorPID(colaDelProceso, pid);
	//TODO mostrar info
}

void mostrarTablaGlobalDeArchivos(){
	printf("Tabla Global de Archivos\n");
	if(list_size(tablaGlobalDeArchivos) > 0){
		int i = 0;
		t_entradaTablaGlobalArchivos* entradaDeLaTablaGlobal;
		printf("Index | Open | Path\n");
		entradaDeLaTablaGlobal = (t_entradaTablaGlobalArchivos*)list_get(tablaGlobalDeArchivos, i);
		printf("path %s\n", entradaDeLaTablaGlobal->path);
		while(entradaDeLaTablaGlobal = (t_entradaTablaGlobalArchivos*)list_get(tablaGlobalDeArchivos, i)){
			printf("     %i |   %i   | %s\n", i, entradaDeLaTablaGlobal->open, entradaDeLaTablaGlobal->path);
			i++;
		}
	}
	else{
		printf("No hay archivos en la tabla global\n");
	}
	printf("\n");
}

void cambiarGradoMultiprogramacion(int gradoNuevo){
	pthread_mutex_lock(&mutexGradoMultiprogramacion);
	grado_multiprog = gradoNuevo;
	pthread_mutex_unlock(&mutexGradoMultiprogramacion);
}

void finalizarProcesoPorPID(int pid, int exitCode){
	t_queue* colaDelProceso = buscarProcesoEnLasColas(pid);
	t_proceso* proceso = obtenerProcesoPorPID(colaDelProceso, pid);
	if(proceso != NULL){
		finalizarProceso(proceso, exitCode);
		printf("El proceso %i se ha finalizado.\n", pid);
	}
	else{
		printf("El proceso no se puede finalizar. Ya ha finalizado o no se encuentra.\n");
	}

}

t_queue* buscarProcesoEnLasColas(int pid){
	t_proceso* proceso;
	if(proceso = obtenerProcesoPorPID(cola_new, pid)){
		return cola_new;
	}
	if(proceso = obtenerProcesoPorPID(cola_ready, pid)){
		return cola_ready;
	}
	if(proceso = obtenerProcesoPorPID(cola_exec, pid)){
		//TODO AVISARLE A CPU QUE LO DEJE DE EJECUTAR
		return cola_exec;
	}
	if(proceso = obtenerProcesoPorPID(cola_block, pid)){
		return cola_block;
	}
	return NULL;
}

t_proceso* obtenerProcesoPorPID(t_queue *cola, int pid){
	int a = 0;
	t_proceso* proceso;
	while(proceso = (t_proceso*)list_get(cola->elements, a)){
		if (proceso->pcb->pid == pid) return (t_proceso*)list_remove(cola->elements, a);
		a++;
	}
	return NULL;
}

int ** desseralizarInstrucciones(t_size instrucciones, t_intructions* instrucciones_serializados){
	int i,j;
	int **indice = malloc(sizeof(int*)*instrucciones);
	for (i=0; i<instrucciones; i++){
		indice[i]= malloc(sizeof(int)*2);
		indice[i][0] = (instrucciones_serializados +1)->start;
		indice[i][1] = (instrucciones_serializados +1)->offset;
	}
	return indice;
}

void reservarHeap(un_socket socketCPU, t_paquete * paqueteRecibido){
	int resultado;
	t_datosHeap* puntero;
	t_proceso *proceso;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec, socketCPU);
	queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&mutex_exec);

	int pid;
	int tamanio;
	tamanio = *((int*)paqueteRecibido->data);
	pid = proceso->pcb->pid;


	if(tamanio > TAMPAG - sizeof(t_heapMetadata)*2){
			// VER EL PROCESO TIENE QUE ABORTAR POR HEAP
		}
	puntero = verificarEspacioLibreHeap(pid, tamanio);
	if(puntero->pagina == -1){
		puntero->pagina = proceso->pcb->paginasDeCodigo + proceso->pcb->paginasDeMemoria + proceso->sizePaginasHeap ;

		//VER mutex memoria?
		resultado = reservarPaginaHeap(pid,puntero->pagina);
		puntero->offset = 8;
		if(resultado <0){
			//ver Excepcion por HEAP
		}
		proceso->sizePaginasHeap++;

	}


	//ver mutex memoria
	resultado = reservarBloqueHeap(pid,tamanio,puntero);

	if(resultado<0){
		// VER NO SE PUDO RESERVAR BLOQUE
	}
	t_direccion* direccion = malloc(sizeof(t_direccion));
	direccion->pagina = puntero->pagina;
	direccion->offset = puntero->offset;
	direccion->size = tamanio;

	enviar(socketCPU, SOLICITAR_HEAP_OK, sizeof(t_direccion), direccion);
	free(puntero);

}

void liberarBloqueHeap(int pid, int pagina, int offset){
	log_info(logger, "Liberando Heap del pid &d", pid);

	int i = 0;
	t_paquete* paquete;
	t_paquete* paquete2;
	t_adminHeap* aux = malloc(sizeof(t_adminHeap));
	t_heapMetadata bloque;

	void* buffer = malloc(sizeof(t_heapMetadata));
	paquete = solicitarBytesHeapMemoria(pid, pagina, offset, sizeof(t_heapMetadata));
	buffer = paquete->data;

	memcpy(&bloque, buffer, sizeof(t_heapMetadata));

	bloque.uso = -1;

	memcpy(buffer, &bloque, sizeof(t_heapMetadata));

	paquete2 = almacenarBytesHeapMemoria(pid, pagina, offset, sizeof(t_heapMetadata), buffer);

	if(paquete2->codigo_operacion == ALMACENAR_BYTES_FALLO){
		log_error(logger, "fallo la liberacion de Heap");
		return;
	}

	while(i<list_size(listaAdminHeap)){
		aux = list_get(listaAdminHeap, i);
		if( aux->pagina == pagina && aux->pid  == pid){
			aux->disponible = aux->disponible + bloque.size;
			list_replace(listaAdminHeap, i, aux);
			break;
		}
		i++;
	}
//ver SEMAFOROS
	liberar_paquete(paquete);
	liberar_paquete(paquete2);
}


int reservarBloqueHeap(int pid, int size, t_datosHeap* puntero){
	log_info(logger,"Reservando bloque en pagina $d del pid $d");
	t_heapMetadata* auxBloque= malloc(sizeof(t_heapMetadata));
	t_adminHeap * aux = malloc(sizeof(t_adminHeap));
	t_paquete* paquete;

	int i=0;
	int sizeLibreViejo;

	void* buffer = malloc(sizeof(t_heapMetadata));

	while(i< list_size(listaAdminHeap)){
		aux = list_get(listaAdminHeap, i);
		if(aux->pagina == puntero->pagina && aux->pid == pid){
			if(size + sizeof(t_heapMetadata) > aux->disponible){
				log_error(logger, "No se pudo reservar el Bloque Heap :( ");
				return -1;
			}
			else{
				aux->disponible = aux->disponible - size - sizeof(t_heapMetadata);
				list_replace(listaAdminHeap, i, aux);
				break;
			}
		}
		i++;
	}

	paquete = solicitarBytesHeapMemoria(pid, puntero->pagina, puntero->offset, sizeof(t_heapMetadata));

	buffer = paquete->data;

	memcpy(&auxBloque, buffer, sizeof(t_heapMetadata));

	sizeLibreViejo = auxBloque->size;
	auxBloque->uso = 1;
	auxBloque->size = size;

	memcpy(buffer, &auxBloque, sizeof(t_heapMetadata));

	t_paquete * respuesta1 = almacenarBytesHeapMemoria(pid,puntero->pagina, puntero->offset, sizeof(t_heapMetadata), buffer);

	if(respuesta1->codigo_operacion==ALMACENAR_BYTES_FALLO){
		log_error(logger, "NO PUDE ALMACENAR DATOS EN BLOQUE");
		return -1;
	}


	auxBloque->uso = -1;
	auxBloque->size = sizeLibreViejo - size - sizeof(t_heapMetadata);

	memcpy(buffer, &auxBloque, sizeof(t_heapMetadata));


	t_paquete * respuesta2 = almacenarBytesHeapMemoria(pid, puntero->pagina, puntero->offset+sizeof(t_heapMetadata)+size,sizeof(t_heapMetadata), buffer);
	if(respuesta2->codigo_operacion==ALMACENAR_BYTES_FALLO){
	log_error(logger, "NO PUDE ALMACENAR DATOS EN BLOQUE");
	return -1;
	}

	log_info(logger, "Bloque de tamanio %d reservado en Heap del pid %d", size, pid);
	free(buffer);
	liberar_paquete(paquete);
	liberar_paquete(respuesta1);
	liberar_paquete(respuesta2);
	return 1;
}


t_datosHeap* verificarEspacioLibreHeap( int pid, int tamanio){
	int i = 0;
	t_datosHeap* puntero = malloc(sizeof(t_datosHeap));
	t_adminHeap * aux;
	puntero->pagina = -1;

	pthread_mutex_lock(&mutex_listaHeap);

	while(i<list_size(listaAdminHeap)){
		aux= (t_adminHeap*) list_get(listaAdminHeap, i);

		if(aux->disponible >= tamanio + sizeof(t_datosHeap) && aux->pid){
			compactarPaginaHeap(aux->pagina, aux->pid);
			puntero->offset = paginaHeapConBloqueSuficiente(i,aux->pagina, aux->pid, tamanio);
			if(puntero->offset>0){
				puntero->pagina=aux->pagina;
				break;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&mutex_listaHeap);
	return puntero;
}

int reservarPaginaHeap(int pid,int pagina){
	log_info(logger, "Reservando pagina %d para el pid %d", pagina, pid);
	int resultado;
	t_heapMetadata* aux;
	void * buffer = malloc(sizeof(t_heapMetadata));

	aux->uso = -1;
	aux->size = TAMPAG - sizeof(t_heapMetadata);

	memcpy(buffer, &aux, sizeof(t_heapMetadata));

	t_pedidoDePaginasKernel* pedido = malloc(sizeof(t_pedidoDePaginasKernel));
	pedido->pid = pid;
	pedido->paginasAPedir = 1;

	enviar(memoria,ASIGNAR_PAGINAS, sizeof(t_pedidoDePaginasKernel), pedido);
	t_paquete* respuesta = recibir(memoria);

	resultado = respuesta->codigo_operacion;

	if(resultado== ASIGNAR_PAGINAS_FALLO) return -1;

	t_paquete * respuesta2 = almacenarBytesHeapMemoria(pid,pagina,0, sizeof(t_heapMetadata), buffer);

	if(respuesta2->codigo_operacion==ALMACENAR_BYTES_OK){
		resultado = 1;
	}else{
		resultado = 0;
	}

	t_adminHeap* admin = malloc(sizeof(t_adminHeap));

	admin->pagina = pagina;
	admin->pid = pid;
	admin->disponible = aux->size;

	list_add(listaAdminHeap, admin);

	free(buffer);
	log_info(logger, "Se reservo la pagina %d para Heap del pid %d", pagina, pid);

	return resultado;

}

void compactarPaginaHeap( int pagina, int pid){

	log_info(logger, "Compactando la pagina %d del Heap del pid %d", pagina, pid);
	int offset = 0;
	t_heapMetadata actual;
	t_heapMetadata siguiente;
	t_heapMetadata* buffer = malloc(sizeof(t_heapMetadata));
	t_paquete * reciboAlgo;
	t_paquete* reciboAlgo2 ;

	actual.size= 0;

	while(offset < TAMPAG && offset + sizeof(t_heapMetadata)+ actual.size> TAMPAG - sizeof(t_heapMetadata)){

		reciboAlgo = solicitarBytesHeapMemoria(pid, pagina, offset, sizeof(t_heapMetadata));

		buffer = (t_heapMetadata*)reciboAlgo->data;

		actual.uso = buffer->uso;
		actual.size = buffer->size;

		reciboAlgo2 = solicitarBytesHeapMemoria(pid, pagina, offset+sizeof(t_heapMetadata)+ actual.size,sizeof(t_heapMetadata));

		buffer = (t_heapMetadata*)reciboAlgo2->data;

		siguiente.uso = buffer->uso;
		siguiente.size = buffer->size;

		if(actual.uso ==-1 && siguiente.uso ==-1){
			actual.size = actual.size + sizeof(t_heapMetadata)+ siguiente.size;
			memcpy(buffer, &actual, sizeof(t_heapMetadata));

			reciboAlgo = almacenarBytesHeapMemoria(pid, pagina, offset, sizeof(t_heapMetadata),(void*)buffer);


		}
		else{
			offset += sizeof(t_heapMetadata)+ actual.size;
		}

	}
	free(buffer);
	liberar_paquete(reciboAlgo);
	liberar_paquete(reciboAlgo2);
	log_info(logger,"Pagina %d del heap del pid %d compactada",pagina, pid);

}


int paginaHeapConBloqueSuficiente(int posicionPaginaHeap, int pagina, int pid, int tamanio){

	int i=0;
	t_heapMetadata auxBloque;
	void* buffer = malloc(sizeof(t_heapMetadata));
	t_paquete* paquete;

	while(i<TAMPAG){
		paquete = solicitarBytesHeapMemoria(pid, pagina, i, sizeof(t_heapMetadata));
		buffer = paquete->data;
		memcpy(&auxBloque, buffer, sizeof(t_heapMetadata));

		if(auxBloque.size >= tamanio + sizeof(t_heapMetadata) && auxBloque.uso == -1){
			log_info(logger,"Pagina %d del heap del pid %d suficiente", pagina, pid);
			free(buffer);
			return i;
		}
		else{
			i = i+sizeof(t_heapMetadata) + auxBloque.size;
		}
	}
	log_error(logger, "Pagina %d del heap del pid %d no suficiente", pagina, pid);
	free(buffer);
	return -1;
}

t_paquete* solicitarBytesHeapMemoria(int pid, int pagina, int offset, int size){

	t_solicitudBytes* solicitud = malloc(sizeof(t_solicitudBytes));
	t_paquete* paquete = malloc(sizeof(t_paquete));
	solicitud->pid = pid;
	solicitud->pagina = pagina;
	solicitud->offset = offset;
	solicitud->tamanio = size;
	enviar(memoria, SOLICITAR_BYTES, sizeof(t_solicitudBytes), solicitud);
	paquete = recibir(memoria);
	return paquete;
}


t_paquete* almacenarBytesHeapMemoria(int pid, int pagina, int offset, int size, void* buffer){
	t_almacenarBytes* almacenar = malloc(sizeof(t_almacenarBytes));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	almacenar->pid =pid;
	almacenar->pagina = pagina;
	almacenar->offset = offset;
	almacenar->tamanio = size;
	almacenar->buffer = buffer;
	enviar(memoria, ALMACENAR_BYTES, sizeof(t_almacenarBytes), almacenar);

	paquete= recibir(memoria);
	return paquete;




}
