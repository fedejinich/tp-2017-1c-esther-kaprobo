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


int cantidadSemaforos = 0;

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
	pthread_mutex_init(&mutexServidor, NULL);
	pthread_mutex_init(&mutexGradoMultiprogramacion, NULL);
	pthread_mutex_init(&mutex_exec, NULL);
	pthread_mutex_init(&mutex_exit, NULL);




	sem_init(&sem_new, 0, 0);
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_cpu, 0, 0);

	//Crear Log
	logger = log_create("kernel.log","Kernel",true,LOG_LEVEL_TRACE);

	//Configuracion
	cargarConfiguracion();
	mostrarConfiguracion();

	//Sockets
	memoria = conectarConLaMemoria();
	fileSystem = conectarConFileSystem();

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
	tablasArchivosPorProceso = list_create();;
}

void cargarConfiguracion() {
	int j;

	bool haySemaforos(){

		bool retorno = false;

		int i;

		for(i=0;i<strlen((char*)sem_inits)/sizeof(char*);i++){

			if(list_size(cola_semaforos[i]->elements)>0) retorno = true;
		}

		return retorno;
	}

	if(hayConfiguracion){
		bool semaforo;


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
	for( j=0; j< strlen((char*)sem_ids) / sizeof(char*);j++){
		cola_semaforos[j] = nalloc(sizeof(t_queue*));
		cola_semaforos[j] = queue_create();
	}

	log_info(logger, "Archivo de configuración cargado");
	hayConfiguracion = true;
	pthread_mutex_unlock(&mutex_config);
}


int* convertirConfigEnInt(char** valores_iniciales){
	int i,b;
	int a = 0;
	int *resul;

	a = (strlen((char*) valores_iniciales))/sizeof(char*);





	int w;

	while(w = valores_iniciales[cantidadSemaforos]){
		cantidadSemaforos++;
	}









	resul=nalloc(((strlen((char*)valores_iniciales))/sizeof(char*))* sizeof(int));

	for (i=0; i< cantidadSemaforos;i++){
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
	log_debug(logger,"Puerto Prog: %i \n", puerto_prog );

	log_debug(logger,"Puerto CPU: %i \n", puerto_cpu);
	log_debug(logger,"IP Memoria: %s \n", ip_memoria);
	log_debug(logger,"Puerto Memoria: %i \n", puerto_memoria);
	log_debug(logger,"IP File System: %s \n", ip_fs);
	log_debug(logger,"Puerto File System: %d \n", puerto_fs);
	log_debug(logger,"Quantum: %i \n", quantum);
	log_debug(logger,"Quantum Sleep: %i \n", quantum_sleep);
	log_debug(logger,"Algoritmo: %i \n", algoritmo);
	log_debug(logger,"Grado Multiprogramacion: %i \n", grado_multiprog);

	//Falta mostrar arrays


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
			pthread_mutex_lock(&mutexServidor);
			paqueteRecibido = recibir(socketCliente[i]);
			pthread_mutex_unlock(&mutexServidor);

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
		nuevoClienteConsola(socketConsola, socketCliente, &numeroClientes);
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

	pthread_mutex_lock(&mutexServidor);
	bool resultado_CPU = esperar_handshake(clientes[*nClientes - 1], HandshakeCPUKernel);
	pthread_mutex_unlock(&mutexServidor);

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

	pthread_mutex_lock(&mutexServidor);
	bool resultado_Consola = esperar_handshake(clientes[*nClientes - 1], HandshakeConsolaKernel);
	pthread_mutex_unlock(&mutexServidor);
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
	log_warning(logger, "CODIGO OP: %i", paqueteRecibido->codigo_operacion);
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
			solicitudDeEscrituraArchivo(socketActivo, paqueteRecibido);

			break;

		case ABRIR_ARCHIVO: //Proceso CPU nos pide abrir un archivo para un proceso dado
			abrirArchivo(socketActivo, paqueteRecibido);
			break;

		case OBTENER_DATOS: //Proceso CPU nos pide leer un archivo para un proceso dado
			leerArchivo(socketActivo, paqueteRecibido);
			break;

		case GUARDAR_DATOS: //Proceso CPU nos pide escribir un archivo para un proceso dado
			break;

		case CERRAR_ARCHIVO: //Proceso CPU nos pide cerrar un archivo para un proceso dado
			cerrarArchivo(socketActivo, paqueteRecibido);
			break;

		case SOLICITAR_VARIABLE: //Proceso CPU nos pide variable compartida
			log_info(logger, "KERNEL: Proceso nos solicita variable compartida");
			solicitaVariable(socketActivo, paqueteRecibido);
			break;

		case ESCRIBIR_VARIABLE:
			log_info(logger, "KERNEL: Proceso nos solicita escribir variable");
			escribirVariable(socketActivo, paqueteRecibido);
			break;

		case SOLICITAR_HEAP:
			log_info(logger,"KERNEL: Proceso nos solicita espacio Dinamico");
			reservarHeap(socketActivo, paqueteRecibido);
			break;

		case PROGRAMA_ABORTADO_CPU:
			log_error(logger, "CPU envio señal de cierre");
			cpuCerrada(socketActivo, paqueteRecibido);


			break;

		case PROGRAMA_FINALIZADO:
			log_info(logger, "Finalizar Programa");
			finalizarProgramaKernel(socketActivo, paqueteRecibido, finalizadoCorrectamente);
			break;


		case LIBERAR_HEAP:
			log_info(logger, "KERNEL: Proceso nos solicita liberar Heap");
			procesoLiberaHeap(socketActivo, paqueteRecibido);
			break;
		case FINALIZAR_PROGRAMA_DESDE_CONSOLA:

			log_info(logger, "Se finalizo PID: %d desde Consola",*(int*)paqueteRecibido->data );
			finalizarProcesoPorPID(*(int*)paqueteRecibido->data, DesconexionDeConsola);
			//VER AVISAR A CPU Y MEMORIA?
			break;

		case ABORTADO_STACKOVERFLOW:
			log_error(logger, "Se finaliza PID %d por STACKOVERFLOW", *(int*)paqueteRecibido->data);
			finalizarProgramaKernel(socketActivo, paqueteRecibido, FaltaDeMemoria);
			//VER FINALIZAR PROGRAMA

			break;

		case EXCEPCION_MEMORIA:
			log_error(logger, "Se finaliza PID %d por Excepcion en Memoria", (int)paqueteRecibido->data);
			finalizarProgramaKernel(socketActivo, paqueteRecibido, ExcepcionDeMemoria);
			break;
		case FIN_QUANTUM:
			log_debug(logger, "Se recibio programa por fin de Quantum");
			finQuantum(socketActivo, paqueteRecibido);
			break;

		case ABORTADO_HEAP:
			finalizarProgramaKernel(socketActivo, paqueteRecibido, IntentoDeReservaDeMemoriaErroneo);
			break;
		case DESCONEXION_CPU:
			log_info(logger, "CPU:%d se desconecto sin estar ejecutando", (int)socketActivo);
			sacarCPUDeListas(socketActivo);
			break;
		case ABORTADO_CONSOLA:
			log_info(logger, "CPU nos envia el PCB serializado que se finalizo ");
			deserializarYFinalizar(socketActivo, paqueteRecibido, DesconexionDeConsola);
			break;
		case ABORTADO_CONSOLA_KERNEL:
			log_info(logger, "CPU nos envia el PCB serializado que se finalizo ");
			deserializarYFinalizar(socketActivo, paqueteRecibido, FinalizacionPorConsolaDeKernel);
			break;

		case MOVER_CURSOR_ARCHIVO:
			log_info(logger, "CPU nos pide mover el cursor de un archivo de un proceso");
			moverCursor(socketActivo, paqueteRecibido);
			break;

		default:
			break;
	}
}


int nuevoProgramaAnsisop(un_socket socket, t_paquete* paquete){
	int exito;
	t_proceso* proceso = malloc(sizeof(t_proceso));
	t_proceso* procesoin = malloc(sizeof(t_proceso));
	proceso = crearPrograma(socket, paquete);
	proceso->socketCPU = -1;

	log_info(logger, "KERNEL: Creando proceso %d", proceso->pcb->pid);


	enviar(proceso->socketConsola, ENVIAR_PID, sizeof(int), &proceso->pcb->pid);


	pthread_mutex_lock(&mutex_new);
	queue_push(cola_new, proceso);
	sem_post(&sem_new); // capaz que no es necesario, para que saque siempre 1, y no haga lio
	pthread_mutex_unlock(&mutex_new);
	pthread_mutex_lock(&mutexGradoMultiprogramacion);
	if(cantidadDeProgramas >= grado_multiprog){

		log_error(logger, "GRADO DE MULTIPROGRAMACION ALCANZADO");
		pthread_mutex_unlock(&mutexGradoMultiprogramacion);

		enviar(proceso->socketConsola, ERROR_MULTIPROGRAMACION, sizeof(int), &proceso->pcb->pid);

		pthread_mutex_lock(&mutex_new);
		t_proceso* proceso = queue_pop(cola_new);
		sem_wait(&sem_new);
		pthread_mutex_unlock(&mutex_new);
		cantidadDeProgramas++;
		finalizarProceso(proceso, ErrorSinDefinicion);

		return -1;
	}
	pthread_mutex_unlock(&mutexGradoMultiprogramacion);

	char* codigo = malloc(paquete->tamanio);
	codigo = paquete->data;
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


		log_error(logger, "KERNEL: SIN ESPACIO EN MEMORIA, se cancela proceso");

		enviar((un_socket)socket, SIN_ESPACIO_MEMORIA, sizeof(int), NULL);

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


void pideSemaforo(un_socket socketActivo, t_paquete* paqueteRecibido){
	t_proceso* procesoPideSem = malloc(sizeof(t_proceso));
	t_paquete* paqueteNuevo = malloc(sizeof(t_paquete));
	t_pcb* pcb_temporal;
	pthread_mutex_lock(&mutex_exec);
	procesoPideSem = obtenerProcesoSocketCPU(cola_exec, socketActivo);
	pthread_mutex_unlock(&mutex_exec);
	eliminarProcesoDeCola(cola_exec, procesoPideSem->pcb->pid);
	pthread_mutex_lock(&mutex_config);

	log_info(logger, "Proceso %d pide semaforo %s", procesoPideSem->pcb->pid, paqueteRecibido->data);

	int* valorSemaforo = buscarSemaforo(paqueteRecibido->data);
	int mandar;

	if(*valorSemaforo<=0){
		mandar =1;
		log_info(logger, "KERNEL: Recibi proceso %d mando a bloquear por semaforo %s", procesoPideSem->pcb->pid, paqueteRecibido->data);

		enviar(socketActivo, PEDIDO_SEMAFORO_FALLO, sizeof(int), &mandar);//1 bloquea proceso
		pthread_mutex_lock(&mutexServidor);
		paqueteNuevo = recibir(socketActivo);
		pthread_mutex_unlock(&mutexServidor);


		pcb_temporal = desserializarPCB(paqueteNuevo->data);
		liberar_paquete(paqueteNuevo);
		destruirPCB(procesoPideSem->pcb);
		procesoPideSem->pcb = pcb_temporal;


		if(procesoPideSem->abortado == false){



			bloqueoSemaforo(procesoPideSem, paqueteRecibido->data);
			queue_push(cola_block, procesoPideSem);

			queue_push(cola_CPU_libres, (void*)socketActivo);
			sem_post(&sem_cpu);

		}else{
			log_info(logger,"KERNEL: Se recibio proceso %d abortado", procesoPideSem->pcb->pid);
			abortar(procesoPideSem);
		}

	}
	else{
		escribeSemaforo(paqueteRecibido->data,*buscarSemaforo(paqueteRecibido->data)-1);
		mandar = 0;
		pthread_mutex_lock(&mutex_exec);
		queue_push(cola_exec, procesoPideSem);
		pthread_mutex_unlock(&mutex_exec);

		enviar(socketActivo, PEDIDO_SEMAFORO_OK, sizeof(int), &mandar);

	}

	pthread_mutex_unlock(&mutex_config);
}

void liberarSemaforo(un_socket socketActivo, t_paquete* paqueteRecibido){

	char* semaforo = (char*)paqueteRecibido->data;
	t_proceso* proceso;
	t_proceso* p1;


	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec, socketActivo);

	pthread_mutex_unlock(&mutex_exec);
	pthread_mutex_lock(&mutex_config);
	log_info(logger, "Proceso %d libera el semaforo %s",proceso->pcb->pid, paqueteRecibido->data);

	int i;




	for(i=0; i< strlen((char*)sem_ids)/sizeof(char*);i++){

		if(strcmp((char*)sem_ids[i],semaforo)==0){

			if((cola_semaforos[i]->elements->elements_count) >0){


				p1 = queue_pop(cola_semaforos[i]);

				pthread_mutex_lock(&mutex_ready);
				queue_push(cola_ready, p1);
				pthread_mutex_unlock(&mutex_ready);



				eliminarProcesoDeCola(cola_block, p1->pcb->pid);

				sem_post(&sem_ready);

			}
			else{

				valor_semaforos[i]++;
			}
			enviar(socketActivo, LIBERAR_SEMAFORO, sizeof(int), &i);

			pthread_mutex_unlock(&mutex_config);
			return;
		}
	}
	log_error(logger, "No encontre el semaforo");
	enviar(socketActivo, LIBERAR_SEMAFORO, sizeof(int), &i);
	pthread_mutex_unlock(&mutex_config);
	return;


}
void bloqueoSemaforo(t_proceso* proceso, char* semaforo){
 	int i;

 	for(i=0; i < cantidadSemaforos;i++){
 		if(strcmp((char*)sem_ids[i], semaforo) == 0){

 			queue_push(cola_semaforos[i], proceso);

 			return;
 		}
 	}
 	log_error(logger,"No encontre el semaforo");

}



void abrirArchivo(un_socket socketActivo, t_paquete* paquete){
	log_info(logger, "Se intentara abrir un archivo");
	char* path = (char*)paquete->data;

	t_paquete* paq2= malloc(sizeof(t_paquete));

	pthread_mutex_lock(&mutexServidor);
	paq2 = recibir(socketActivo);
	pthread_mutex_unlock(&mutexServidor);

	int pid = paq2->codigo_operacion;
	char* permisos = (char*)paq2->data;

	log_info(logger, "EL PID: %d, nos solicita abrir el archivo %s, con permisos %s", pid, path, permisos);

	if(existeArchivo(path)){
		int* fdAMandar;
		fdAMandar = agregarNuevoArchivoATablas(pid, path, permisos);
		enviar(socketActivo, ABRIR_ARCHIVO, sizeof(int), &fdAMandar);
	}
	else{
		log_info(logger, "Se intentara Crear el archivo");

		if(string_contains(permisos, "c")){

			enviar(fileSystem, CREAR_ARCHIVO, strlen(path) + 1, path);

			pthread_mutex_lock(&mutexServidor);
			t_paquete* paqueteResultadoCreacion = recibir(fileSystem);
			pthread_mutex_unlock(&mutexServidor);

			if(paqueteResultadoCreacion->codigo_operacion == CREAR_ARCHIVO_OK){
				log_debug(logger, "Se creo el archivo correctamente");

				int fdAMandar = agregarNuevoArchivoATablas(pid, path, permisos);

				enviar(socketActivo, ABRIR_ARCHIVO, sizeof(int), &fdAMandar);
			}
			else{
				log_warning(logger, "No se pudo crear el archivo");
				finalizarProcesoPorPID(pid, ErrorSinDefinicion );
				enviar(socketActivo, ARCHIVO_NO_SE_PUDO_ABRIR, sizeof(int), &pid);
				return;
			}
		}
		else{
			log_info(logger, "El proceso no tiene permisos para crear archivo");
			finalizarProcesoPorPID(pid, ArchivoInexistente);
			enviar(socketActivo, ARCHIVO_NO_SE_PUDO_ABRIR, sizeof(int), &pid);
			return;

		}
	}
}

int* agregarNuevoArchivoATablas(int pid, char* path, char* permisos){
	int* retorno;
	log_info(logger, "Agrego un nuevo archivo del proceso PID: %i", pid);
	t_entradaTablaProceso* entradaLocal = malloc(sizeof(t_entradaTablaProceso));

	entradaLocal->fd = fdcounter;
	fdcounter++;
	entradaLocal->flags = permisos;
	entradaLocal->globalFD = chequearTablaGlobal(path);
	entradaLocal->puntero = 0;

	//Agrego la entrada a la tabla en el indice = pid, que es la tabla correspondiente
	t_entradaTablasArchivosPorProceso* entradaTablaDelProceso = obtenerTablaDeArchivosDeUnProcesoPorPID(pid);
	if(entradaTablaDelProceso == NULL){
		entradaTablaDelProceso = crearTablaDeArchivosDeUnProceso(pid);
	}

	list_add(entradaTablaDelProceso->tablaDeUnProceso, entradaLocal);
	retorno = entradaLocal->fd;
	return retorno;
}

t_entradaTablasArchivosPorProceso* crearTablaDeArchivosDeUnProceso(int pid){
	log_info(logger, "Creo la tabla de archivos para el proceso PID: %i", pid);
	t_entradaTablasArchivosPorProceso* nuevaTabla = malloc(sizeof(t_entradaTablasArchivosPorProceso));
	nuevaTabla->pid = pid;
	nuevaTabla->tablaDeUnProceso = list_create();
	list_add(tablasArchivosPorProceso, nuevaTabla);

	return nuevaTabla;
}

void solicitudDeEscrituraArchivo(un_socket socketActivo, t_paquete* paqueteRecibido){

	t_escribirArchivo* escritura = malloc(sizeof(t_escribirArchivo));

	memcpy(escritura, paqueteRecibido->data, sizeof(t_escribirArchivo));
	pthread_mutex_lock(&mutexServidor);
	t_paquete* info = recibir(socketActivo);
	pthread_mutex_unlock(&mutexServidor);


	if(escritura->fd==1){
		log_info(logger,"KERNEL: Proceso %d nos solicita imprimir texto por Consola", escritura->pid);

		log_debug(logger, "Informacion a mostrar: %s", (char*)info->data);

		t_proceso* proceso;
		pthread_mutex_lock(&mutex_exec);
		proceso = obtenerProcesoSocketCPU(cola_exec, socketActivo);

		pthread_mutex_unlock(&mutex_exec);

		int basura;
		if(proceso->abortado==false){
			enviar((un_socket)(proceso->socketConsola), IMPRIMIR_CONSOLA, escritura->size, info->data);
			enviar(socketActivo, ESCRIBIR_ARCHIVO_OK, sizeof(int), basura);
		}
		else{
			enviar(socketActivo, ESCRIBIR_ARCHIVO_FALLO, sizeof(int), (void*)basura);
		}
	}
	else{
		//VER ESCRIBIR ARCHIVO
		escribirArchivo(socketActivo, escritura->pid, escritura->fd, escritura->size, (char*)info->data);
	}



}

void escribirArchivo(un_socket socketActivo, int pid, t_descriptor_archivo fd, int size, char* buffer){



	//Obtengo la tabla del proceso PID
	t_entradaTablasArchivosPorProceso* tablaDeUnProceso = malloc(sizeof(t_entradaTablasArchivosPorProceso));
	tablaDeUnProceso = obtenerTablaDeArchivosDeUnProcesoPorPID(pid);

	if(tablaDeUnProceso == NULL){
		//FINALIZAR POR ERROR DESCONOCIDO, NO EXISTE/ENCONTRO LA TABLA
	}

	log_debug(logger, "YA TENGO LA TABLA DEL PROCESO %i", tablaDeUnProceso->pid);

	//Busco el archivo en la tabla del proceso
	log_debug(logger, "CANTIDAD DE ARCHIVOS EN LA TABLA DEL PROCESO: %i", tablaDeUnProceso->tablaDeUnProceso->elements_count);
	t_entradaTablaProceso* archivo = obtenerArchivoDeLaTablaDeUnProcesoPorFD(tablaDeUnProceso, fd);

	log_debug(logger, "TENGO EL ARCHIVO %i", archivo->fd);

	char* permisos = archivo->flags;

	//Path donde debe escribirse
	t_entradaTablaGlobal* entradaTablaGlobal = obtenerEntradaTablaGlobalDeArchivos(archivo);
	char* path = entradaTablaGlobal->path;

	log_info(logger, "CHEQUEO PERMISOS ESCRITURA");

	if(strchr(permisos, 'w') != NULL && existeArchivo(path)){
		log_info(logger, "CON PERMISO");
		t_pedidoGuardadoDatos* guardadoDatos = malloc(sizeof(t_pedidoGuardadoDatos));

		//Datos Para la escritura
		guardadoDatos->offset = archivo->puntero;
		guardadoDatos->size = size;

		log_info(logger, "ENVIO DATOS PARA ESCRITURA");
		//Hago 3 envios en orden: Datos para guardado de info, path donde debe guardarse, informacion a guardar
		enviar(fileSystem, GUARDAR_DATOS, sizeof(t_pedidoGuardadoDatos), guardadoDatos);
		enviar(fileSystem, GUARDAR_DATOS, strlen(path) + 1, path);
		enviar(fileSystem, GUARDAR_DATOS, strlen(buffer) + 1, buffer);



		//Espero la respuesta del guardado de info
		pthread_mutex_lock(&mutexServidor);
		t_paquete* paquete = recibir(fileSystem);
		pthread_mutex_unlock(&mutexServidor);
		//Aviso a CPU del resultado del guardado de info
		int* basura = 1;
		if(paquete->codigo_operacion == SOLICITUD_GUARDADO_DATOS_OK){
			log_warning(logger, "ESCRITURA OK");
			enviar(socketActivo, ESCRIBIR_ARCHIVO_OK, sizeof(int), &basura);
		}
		else{
			log_warning(logger, "ESCRITURA FALLO");
			finalizarProcesoPorPID(pid, ErrorSinDefinicion);
		}
	}
	else{
		log_info(logger, "SIN PERMISO");
		finalizarProcesoPorPID(pid, IntentoDeEscrituraSinPermisos);
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
			finalizarProcesoPorPID(pid, ArchivoInexistente);
		}
	}
	return permisoParaSeguir;
}

bool existeArchivo(char* path){
	log_info(logger, "Verifico que el archivo exista en FS");
	bool resultado = false;

	enviar(fileSystem, VALIDAR_ARCHIVO, strlen(path)+1, path);

	pthread_mutex_lock(&mutexServidor);
	t_paquete* paqueteResultado = recibir(fileSystem);
	pthread_mutex_unlock(&mutexServidor);

	if(paqueteResultado->codigo_operacion == VALIDAR_ARCHIVO_OK){
		resultado = true;
		log_info(logger, "El archivo existe en FS");
		return resultado;
	}

	log_info(logger, "El archivo no existe en FS");
	return resultado;
}

int chequearTablaGlobal(char* path){
	int fd;
	int indiceEntrada = buscarEntradaEnTablaGlobal(path);
	if(indiceEntrada != -1){
		fd = indiceEntrada;
	}
	else{
		t_entradaTablaGlobal* entrada = malloc(sizeof(t_entradaTablaGlobal));
		entrada->path = path;
		entrada->open = 0;
		list_add(tablaGlobalDeArchivos, entrada);
		fd = buscarEntradaEnTablaGlobal(path);
	}
	return fd;
}

int buscarEntradaEnTablaGlobal(char* path){
	int a;
	t_entradaTablaGlobal* entrada;
	while(entrada = (t_entradaTablaGlobal*)list_get(tablaGlobalDeArchivos, a)){
		if (entrada->path == path){
			entrada->open++;
			return a;
		}
		a++;
	}
	return -1;
}



void leerArchivo(un_socket socketActivo, t_paquete* paquete){
	t_envioDeDatosKernelFSLecturaYEscritura* datos= (t_envioDeDatosKernelFSLecturaYEscritura*)paquete->data;
	int pid = datos->pid;
	int fd = datos->fd;

	log_info(logger, "Me piden leer pid:%d, fd:%d, size:%d, offset:%d", datos->pid,datos->fd,datos->tamanio,datos->offset);



	//Obtengo la tabla del proceso PID
	t_entradaTablasArchivosPorProceso* tablaDeUnProceso = malloc(sizeof(t_entradaTablasArchivosPorProceso));
	tablaDeUnProceso = obtenerTablaDeArchivosDeUnProcesoPorPID(pid);

	if(tablaDeUnProceso == NULL){
		//FINALIZAR POR ERROR DESCONOCIDO, NO EXISTE/ENCONTRO LA TABLA
	}

	log_warning(logger, "YA TENGO LA TABLA DEL PROCESO %i", tablaDeUnProceso->pid);

	//Busco el archivo en la tabla del proceso
	log_warning(logger, "CANTIDAD DE ARCHIVOS EN LA TABLA DEL PROCESO: %i", tablaDeUnProceso->tablaDeUnProceso->elements_count);
	t_entradaTablaProceso* archivo = obtenerArchivoDeLaTablaDeUnProcesoPorFD(tablaDeUnProceso, fd);

	char* permisos = archivo->flags;

	//Path donde debe escribirse
	t_entradaTablaGlobal* entradaTablaGlobal = obtenerEntradaTablaGlobalDeArchivos(archivo);
	char* path = entradaTablaGlobal->path;

	if(string_contains(permisos,"r")  && existeArchivo(path)){
		t_pedidoGuardadoDatos* obtencionDatos = malloc(sizeof(t_pedidoGuardadoDatos));

		//Datos Para la escritura
		obtencionDatos->offset = 0;
		obtencionDatos->size = datos->tamanio;

		enviar(fileSystem, SOLICITUD_OBTENCION_DATOS, sizeof(t_pedidoGuardadoDatos), obtencionDatos);
		enviar(fileSystem, SOLICITUD_OBTENCION_DATOS, strlen(path) + 1, path);

		pthread_mutex_lock(&mutexServidor);
		t_paquete* paquete = recibir(fileSystem);
		pthread_mutex_unlock(&mutexServidor);

		//Aviso a CPU del resultado del guardado de info
		int basura = malloc(sizeof(int));
		if(paquete->codigo_operacion == SOLICITUD_OBTENCION_DATOS_OK){
			char* datosLeidos = malloc(paquete->tamanio);
			datosLeidos = (char*)(paquete->data);
			enviar(socketActivo, SOLICITUD_OBTENCION_DATOS_OK, paquete->tamanio, datosLeidos);
		}
		else{
			//DEBERIA ABORTAR? O HACER ESTO DE AVISARLE A CPU
			//PARA MI DEBERIA ABORTAR
			finalizarProcesoPorPID(pid, ErrorSinDefinicion);
			enviar(socketActivo, SOLICITUD_OBTENCION_DATOS_FALLO, sizeof(int), &pid);
		}
	}
	else{
		enviar(socketActivo, SOLICITUD_OBTENCION_DATOS_FALLO, sizeof(int), &pid);
		finalizarProcesoPorPID(pid, IntentoDeLecturaSinPermisos);
	}
}

void cerrarArchivo(un_socket socketActivo, t_paquete* paquete){
	log_warning(logger, "CERRANDO ARCHIVO");
	t_pedidoGuardadoDatos* datos= (t_pedidoGuardadoDatos*)paquete->data;
	int pid = datos->offset;
	int fd = datos->size;

	t_entradaTablaProceso* entradaTablaProceso = obtenerEntradaTablaArchivosDelProceso(pid, fd);
	t_entradaTablaGlobal* entradaTablaGlobal = obtenerEntradaTablaGlobalDeArchivos(entradaTablaProceso);

	if(entradaTablaGlobal->open > 1){
		log_warning(logger, "ESTABA ABIERTO MAS DE UNA VEZ");
		//Borrar la entrada de la tabla del proceso
		borrarArchivoDeTabla(pid, fd);

		//Actualizo el open
		entradaTablaGlobal->open--;
	}
	else{
		log_warning(logger, "ESTABA ABIERTO SOLO UNA VEZ");
		//Borrar la entrada de la tabla del proceso
		borrarArchivoDeTabla(pid, fd);
		log_warning(logger, "Archivo %i eliminado de la tabla del proceso %i", fd, pid);

		//Borrar la entrada de la tabla global
		list_remove(tablaGlobalDeArchivos, entradaTablaProceso->globalFD);
		log_warning(logger, "Archivo %i eliminado de la tabla de archivos globales", entradaTablaProceso->globalFD);
	}
	//int basura;
	enviar(socketActivo, CERRAR_ARCHIVO, sizeof(int), &pid);
}

void borrarArchivo(int* socketActivo, t_paquete* paquete){
	char* path = paquete->data;

	int i;
	int indiceAEliminar;
	t_entradaTablaGlobal* entradaDeLaTablaGlobal = malloc(sizeof(t_entradaTablaGlobal));
	t_entradaTablaGlobal* entradaAEliminar= malloc(sizeof(t_entradaTablaGlobal));

	while(entradaDeLaTablaGlobal = (t_entradaTablaGlobal*)list_get(tablaGlobalDeArchivos, i)){
		if(entradaDeLaTablaGlobal->path == path){
			entradaAEliminar = entradaDeLaTablaGlobal;
			indiceAEliminar = i;
		}
		i++;
	}

	if(entradaAEliminar->open == 0){

		enviar(socketActivo, BORRAR_ARCHIVO, strlen(path) + 1, path);

		pthread_mutex_lock(&mutexServidor);
		t_paquete* paquete = recibir(fileSystem);
		pthread_mutex_unlock(&mutexServidor);

		int basura;
		if(paquete->codigo_operacion == BORRAR_ARCHIVO_OK){
			list_remove(tablaGlobalDeArchivos, indiceAEliminar);
		}
		enviar(socketActivo, paquete->codigo_operacion, sizeof(int), basura);
	}
}

t_entradaTablaProceso* obtenerEntradaTablaArchivosDelProceso(int pid, int fd){
	t_entradaTablasArchivosPorProceso* tablaDeUnProceso = obtenerTablaDeArchivosDeUnProcesoPorPID(pid);
	t_entradaTablaProceso* entradaTablaDelProceso = obtenerArchivoDeLaTablaDeUnProcesoPorFD(tablaDeUnProceso, fd);

	return entradaTablaDelProceso;
}

t_entradaTablaGlobal* obtenerEntradaTablaGlobalDeArchivos(t_entradaTablaProceso* entradaTablaDelProceso){
	return list_get(tablaGlobalDeArchivos, entradaTablaDelProceso->globalFD);
}

void borrarArchivoDeTabla(int pid, int fd){
	t_entradaTablasArchivosPorProceso* tablaDeUnProceso = obtenerTablaDeArchivosDeUnProcesoPorPID(pid);
	eliminarArchivoDeTabla(tablaDeUnProceso, fd);
}

void eliminarArchivoDeTabla(t_entradaTablasArchivosPorProceso* tablaDeUnProceso, t_descriptor_archivo fd){
	int i = 0;
	int aEliminar;
	t_entradaTablaProceso* archivo = malloc(sizeof(t_entradaTablaProceso));

	while(archivo = (t_entradaTablaProceso*)list_get(tablaDeUnProceso->tablaDeUnProceso, i)){
		if(archivo->fd == fd){
			aEliminar = i;
		}
		i++;
	}
	list_remove(tablaDeUnProceso->tablaDeUnProceso, aEliminar);
}

t_entradaTablasArchivosPorProceso* obtenerTablaDeArchivosDeUnProcesoPorPID(int pid){
	int i = 0;
	t_entradaTablasArchivosPorProceso* tabla = malloc(sizeof(t_entradaTablasArchivosPorProceso));

	while(tabla = (t_entradaTablasArchivosPorProceso*)list_get(tablasArchivosPorProceso, i)){
		if(tabla->pid == pid){
			return tabla;
		}
		i++;
	}
}

t_entradaTablaProceso* obtenerArchivoDeLaTablaDeUnProcesoPorFD(t_entradaTablasArchivosPorProceso* tabla, t_descriptor_archivo fd){
	int i = 0;
	t_entradaTablaProceso* archivo = malloc(sizeof(t_entradaTablaProceso));

	while(archivo = (t_entradaTablaProceso*)list_get(tabla->tablaDeUnProceso, i)){
		if(archivo->fd == fd){
			return archivo;
		}
		i++;
	}
	log_error(logger, "SALI SIN NADA");
	return NULL;
}

void moverCursor(un_socket socketActivo, t_paquete* paqueteRecibido){
	t_moverCursor* mover = malloc(sizeof(t_moverCursor));
	mover = (t_moverCursor*)paqueteRecibido->data;
	t_entradaTablasArchivosPorProceso* tablaDeUnProceso = obtenerTablaDeArchivosDeUnProcesoPorPID(mover->pid);
	t_entradaTablaProceso* entradaTablaDelProceso = obtenerArchivoDeLaTablaDeUnProcesoPorFD(tablaDeUnProceso, mover->fd);

	entradaTablaDelProceso->puntero = mover->posicion;

	int basura = 1;
	enviar(socketActivo, MOVER_CURSOR_ARCHIVO_OK, sizeof(int), &basura);
	free(mover);
}

void solicitaVariable(un_socket socketActivo, t_paquete* paqueteRecibido){
	int* valor;
	t_proceso* proceso;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec,socketActivo);

	pthread_mutex_unlock(&mutex_exec);
	pthread_mutex_lock(&mutex_config);
	valor = valorVariable(paqueteRecibido->data);


	enviar((un_socket)socketActivo,SOLICITAR_VARIABLE_OK, sizeof(int), &valor);
	pthread_mutex_unlock(&mutex_config);
	return;
}

int* valorVariable(char* variable){
	char* aux = malloc(strlen(variable)+2);
	aux[0]= '!';
	int j;
	for(j=0; j<strlen(variable);j++){
		aux[j+1] = variable[j];
	}

	aux[j+1] = '\0';

	int i;
	log_info(logger, "Se solicita variable %s", aux);
	for(i=0; i < strlen((char*)shared_vars)/ sizeof(char*); i++){
		if(strcmp((char*)shared_vars[i], aux)==0){
			return valor_shared_vars[i];
		}
	}
	log_error(logger, "KERNEL, no se encontro variable %s, exit", variable);
	exit(0);
}

void escribirVariable(un_socket socketActivo, t_paquete* paqueteRecibido){

	t_paquete* paq1;

	char* nombre = malloc(paqueteRecibido->tamanio);
	nombre = paqueteRecibido->data;
	pthread_mutex_lock(&mutexServidor);
	paq1 = recibir(socketActivo);
	pthread_mutex_unlock(&mutexServidor);
	int valor2 = *(int*)paq1->data;

	char* aux = malloc(strlen(nombre)+2);
	aux[0]= '!';

	int j;
	for(j=0; j<strlen(nombre);j++){
		aux[j+1] = nombre[j];
	}
	aux[j+1] = '\0';
	log_info(logger,"Se va a escribir la variable %s con el valor %d", aux, valor2);
	free(nombre);

	t_proceso* proceso;

	int i;

	pthread_mutex_lock(&mutex_exec);
	proceso=obtenerProcesoSocketCPU(cola_exec, socketActivo);

	pthread_mutex_unlock(&mutex_exec);
	pthread_mutex_lock(&mutex_config);

	for(i=0; i<strlen((char*)shared_vars)/sizeof(char*);i++){
		if(strcmp((char*)shared_vars[i], aux)==0){
			valor_shared_vars[i] = valor2;
			pthread_mutex_unlock(&mutex_config);
			return;
		}
	}
	log_error(logger, "No se encontro Variable %s, EXIT",aux);
	pthread_mutex_unlock(&mutex_config);
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

t_proceso* crearPrograma(un_socket socketC , t_paquete* paquete){
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
	log_info(logger,"Intrucciones: %d \n", metadata->instrucciones_size);

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

		pthread_mutex_lock(&mutexServidor);
		paquete = recibir(socketMemoria);
		pthread_mutex_unlock(&mutexServidor);
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
	pthread_mutex_lock(&mutexServidor);
	t_paquete * paquete = recibir(memoria);
	pthread_mutex_unlock(&mutexServidor);

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
		tamanioCodigoParcial++; // por el \0

		int pagina = i;
		int offset = 0;

		realloc(codigoParcial, tamanioCodigoParcial);
		strcpy(codigoParcial + tamanioCodigoParcial - 1, "\0");
		log_warning(logger, "Codigo parcial %s", codigoParcial);
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
			log_error(logger,"PLANIF DETENIDA");
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
/*
	pthread_mutex_lock(&mutex_exec);
	queue_pop(cola_exec);
	pthread_mutex_unlock(&mutex_exec);
	*/
}



t_proceso* obtenerProcesoSocketCPU(t_queue *cola, un_socket socketBuscado){



	int a = 0;
	t_proceso*proceso;



	while(a< list_size(cola->elements)){
		proceso = list_get(cola->elements, a);

		if(proceso->socketCPU == socketBuscado){

			return proceso;
		}

		a++;
	}



	log_error(logger, "No hay proceso para retirar");

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
		t_entradaTablaGlobal* entradaDeLaTablaGlobal;
		printf("Index | Open | Path\n");
		entradaDeLaTablaGlobal = (t_entradaTablaGlobal*)list_get(tablaGlobalDeArchivos, i);
		printf("path %s\n", entradaDeLaTablaGlobal->path);
		while(entradaDeLaTablaGlobal = (t_entradaTablaGlobal*)list_get(tablaGlobalDeArchivos, i)){
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


void abortar(t_proceso* proceso){
	enviar(proceso->socketConsola, ABORTADO_CPU, sizeof(int), &proceso->pcb->pid);
	pthread_mutex_lock(&mutex_exit);
	destruirCONTEXTO(proceso->pcb);
	queue_push(cola_exit, proceso);
	pthread_mutex_unlock(&mutex_exit);

	enviar(memoria, FINALIZAR_PROCESO, sizeof(int), &proceso->pcb->pid);

}

void finalizarProgramaKernel(un_socket socket, t_paquete* paquete, ExitCodes exitCode){
/*
	bool esMiCPU(void* entrada){
		t_proceso* proc = (t_proceso*) entrada;
		return proc->socketCPU == socket;
	}*/
 	t_proceso* proceso;

 	pthread_mutex_lock(&mutex_exec);
 	proceso= obtenerProcesoSocketCPU(cola_exec, socket);
 	pthread_mutex_unlock(&mutex_exec);

 	eliminarProcesoDeCola(cola_exec, proceso->pcb->pid);


 	/*
 	if(proceso==NULL){
		int i;
		for (i=0; i < strlen((char*)sem_ids)/sizeof(char*); i++){
				proceso = (t_proceso*)list_remove_by_condition(cola_semaforos[i]->elements, esMiCPU);
				if(proceso!=NULL){
					log_debug(logger, "El proceso se encontraba bloqueado por Semaforo");

				}
			}

 	}*/

 	finalizarProceso(proceso, exitCode);


 }

void finalizarProceso(t_proceso* proceso, ExitCodes exitCode){
	/*if(exitCode == DesconexionDeConsola){
		proceso->abortado = true;


	}
*/
	proceso->pcb->exitCode = exitCode;

	log_info(logger, "Se finalizara proceso %d por Exit Code %d", (int)proceso->pcb->pid, (int)exitCode);
	int i = 0;

	t_adminHeap* aux = malloc(sizeof(t_adminHeap));

	int sizeAux = list_size(listaAdminHeap);
	while(i < sizeAux){
		log_warning(logger, "if(list_size(listaAdminHeap): %i == 0", list_size(listaAdminHeap));
		if(list_size(listaAdminHeap) == 0){
			i++;
		} else {

			log_warning(logger, "list_get(listaAdminHeap, %i)", i);
			aux = list_get(listaAdminHeap, i);


			if(aux != NULL) {
				log_warning(logger, "aux->pid: %i == (proceso->pcb->pid): %i", aux->pid, proceso->pcb->pid);
				if(aux->pid  == (proceso->pcb->pid)){
					//t_adminHeap* aux2 = malloc(sizeof(t_adminHeap));
					/*aux2 =*/ list_remove_and_destroy_element(listaAdminHeap, i, free);
					log_debug(logger, "Se elimina el registro en heap de pagina %d del PID %d ", aux->pagina, aux->pid);
					//free(aux2);
					/* VER ESTO
					i++;
					/* VER ESTO*/
				} else {
					i++;
				}

			}
		}
		i++;
	}


	pthread_mutex_lock(&mutex_exit);
	destruirCONTEXTO(proceso->pcb);
	queue_push(cola_exit, proceso);
	pthread_mutex_unlock(&mutex_exit);


	if(proceso->socketCPU >0){
		queue_push(cola_CPU_libres, (void*)proceso->socketCPU);
		sem_post(&sem_cpu);

	}

	enviar(memoria,FINALIZAR_PROCESO, sizeof(int), &proceso->pcb->pid);
	log_debug(logger, "Se finalizo PID en Memoria");
	if(exitCode != DesconexionDeConsola){
		enviar(proceso->socketConsola, FINALIZAR_PROGRAMA, sizeof(int), &proceso->pcb->pid);
		log_debug(logger, "Se finalizo PID en CONSOLA");
	}

	pthread_mutex_lock(&mutexGradoMultiprogramacion);
	cantidadDeProgramas--;
	pthread_mutex_unlock(&mutexGradoMultiprogramacion);
	return;

}




void finalizarProcesoPorPID(int pid, int exitCode){



//sleep(1);
	t_proceso* proceso;
	t_queue* colaDelProceso = buscarProcesoEnLasColas(pid);
	if(colaDelProceso == NULL){


		proceso = verSiEstaBloqueado(pid);
	}
	else{
		if((int)colaDelProceso ==1){

			proceso = obtenerProcesoPorPID(cola_exec, pid);


			if(exitCode == FinalizacionPorConsolaDeKernel){

				enviar(proceso->socketCPU, ABORTADO_CONSOLA_KERNEL, sizeof(int),&proceso->pcb->pid);
				proceso = NULL;
			}
			else{
				if(exitCode == DesconexionDeConsola){

					enviar(proceso->socketCPU, ABORTADO_CONSOLA, sizeof(int),&proceso->pcb->pid);
					proceso=NULL;
				}
			}
			//eliminarProcesoDeCola(cola_exec, pid);


		}
		else{

			pthread_mutex_lock(&mutex_new);
			pthread_mutex_lock(&mutex_ready);
			proceso = eliminarProcesoDeCola(colaDelProceso, pid);
			pthread_mutex_unlock(&mutex_new);
			pthread_mutex_unlock(&mutex_ready);
		}
	}

	if(proceso != NULL){

		finalizarProceso(proceso, exitCode);

	}
	else{
		log_warning(logger,"El proceso %d ya fue finalizado o no se encuentra.\n", pid);
	}
	return;

}


t_proceso* verSiEstaBloqueado(int pid){
	int i;

	t_proceso* proc = malloc(sizeof(t_proceso));
	for (i=0; i < cantidadSemaforos; i++){

		proc = (t_proceso*)list_get(cola_semaforos[i]->elements,i);


		if(proc!=NULL){

			if((proc->pcb->pid) == pid){
				log_debug(logger, "El proceso se encontraba bloqueado por Semaforo");
				proc = (t_proceso*)list_remove(cola_semaforos[i]->elements,i);
				eliminarProcesoDeCola(cola_block, pid);

				return proc;
			}

		}
	}
	return NULL;

}

t_queue* buscarProcesoEnLasColas(int pid){
	t_proceso* proceso;

	pthread_mutex_lock(&mutex_new);
	proceso = obtenerProcesoPorPID(cola_new, pid);
	if(proceso != NULL){

		pthread_mutex_unlock(&mutex_new);
		return cola_new;
	}
	pthread_mutex_unlock(&mutex_new);

	pthread_mutex_lock(&mutex_ready);
	proceso = obtenerProcesoPorPID(cola_ready, pid);
	if(proceso != NULL){

		pthread_mutex_unlock(&mutex_ready);
		return cola_ready;
	}
	pthread_mutex_unlock(&mutex_ready);




	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoPorPID(cola_exec, pid);
	if(proceso!= NULL){

		pthread_mutex_unlock(&mutex_exec);


		return 1;

	}
	pthread_mutex_unlock(&mutex_exec);

	return NULL;
}

t_proceso* eliminarProcesoDeCola(t_queue* cola, int pid){
	int a = 0;
	t_proceso* proceso;

	while(a<list_size(cola->elements)){
		proceso = list_get(cola->elements, a);
		if (proceso->pcb->pid == pid){
			proceso = list_remove(cola->elements, a);
			return proceso;
		}

		a++;
	}
	return NULL;

}

t_proceso* obtenerProcesoPorPID(t_queue *cola, int pid){
	int a = 0;
	t_proceso* proceso;

	while(a<list_size(cola->elements)){
		proceso = list_get(cola->elements, a);
		if (proceso->pcb->pid == pid)
			return proceso;
		a++;
	}
	return NULL;
}

int ** desseralizarInstrucciones(t_size instrucciones, t_intructions* instrucciones_serializados){
	int i;
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
	t_pedidoHeap* pedido;
	t_datosHeap* puntero;
	t_proceso *proceso;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec, socketCPU);
	//queue_push(cola_exec, proceso);
	pthread_mutex_unlock(&mutex_exec);

	int pid;
	int tamanio;

	pedido = paqueteRecibido->data;
	tamanio = pedido->tamanio;
	pid = pedido->pid;

	log_info(logger,"PROCESO %d nos pide HEAP\n\n", pid);
	if(tamanio > TAMPAG - sizeof(t_heapMetadata)*2){
			log_error(logger, "ERROR, Se intento Reservar mas memoria que el tamanio de una pagina ");


			enviar(socketCPU, SOLICITAR_HEAP_FALLO, sizeof(int), &pid);

			//finalizarProceso(proceso, IntentoDeReservaDeMemoriaErroneo);

			return;
		}
	puntero = verificarEspacioLibreHeap(pid, tamanio);
	if((puntero->pagina==EXIT_FAILURE_CUSTOM) &&(puntero->offset==EXIT_FAILURE_CUSTOM)){

		enviar(socketCPU, SOLICITAR_HEAP_FALLO, sizeof(int), &pid);

		log_warning(logger, "No hay espacio para Heap");


		return;
	}
	if(puntero->pagina == -1){
		puntero->pagina = proceso->pcb->paginasDeCodigo + proceso->pcb->paginasDeMemoria + proceso->sizePaginasHeap +1 ;

		//VER mutex memoria?
		resultado = reservarPaginaHeap(pid,puntero->pagina);
		puntero->offset = 0;
		if(resultado <0){

			enviar(socketCPU, SOLICITAR_HEAP_FALLO, sizeof(int), &pid);
			return;
		}
		proceso->sizePaginasHeap++;

	}



	//ver mutex memoria
	resultado = reservarBloqueHeap(pid,tamanio,puntero);

	if(resultado<0){
		finalizarProceso(proceso, ExcepcionDeMemoria);
	}
	t_direccion* direccion = malloc(sizeof(t_direccion));
	direccion->pagina = puntero->pagina;
	direccion->offset = puntero->offset;
	direccion->size = tamanio;

	enviar(socketCPU, SOLICITAR_HEAP_OK, sizeof(t_direccion), direccion);
	free(puntero);


}


void procesoLiberaHeap(un_socket socketCPU, t_paquete * paqueteRecibido){
	t_liberarHeap * libera = malloc(sizeof(t_liberarHeap));

	libera = (t_liberarHeap*)(paqueteRecibido->data);
	codigosKernelCPU codigo = liberarBloqueHeap(libera->pid, libera->nroPagina, libera->offset);
	void* algo = malloc(sizeof(int));

	enviar(socketCPU, codigo, sizeof(int), algo);
	free(algo);

}

codigosKernelCPU liberarBloqueHeap(int pid, int pagina, int offset){
	log_info(logger, "Liberando Heap del pid %d", pid);

	int i = 0;

	t_adminHeap* aux = malloc(sizeof(t_adminHeap));
	t_adminHeap* aux2 = malloc(sizeof(t_adminHeap));
	t_heapMetadata * bloque= malloc(sizeof(t_heapMetadata));


	bloque = (t_heapMetadata*)solicitarBytesAMemoria(memoria, logger, pid, pagina, offset, sizeof(t_heapMetadata));



	bloque->uso = 0;

	int resul= almacenarEnMemoria(memoria, logger, pid, pagina, offset, sizeof(t_heapMetadata), bloque);

	bloque = (t_heapMetadata*)solicitarBytesAMemoria(memoria, logger, pid, pagina, offset, sizeof(t_heapMetadata));



	if (resul == EXIT_FAILURE_CUSTOM){
		return LIBERAR_HEAP_FALLO;
	}

	while(i<list_size(listaAdminHeap)){
		aux = list_get(listaAdminHeap, i);
		if( aux->pagina == pagina && aux->pid  == pid){
			aux->disponible = aux->disponible + bloque->size;
			list_replace(listaAdminHeap, i, aux);
			aux2=list_get(listaAdminHeap, i);

			break;
		}
		i++;
	}

	return LIBERAR_HEAP_OK;
}


int reservarBloqueHeap(int pid, int size, t_datosHeap* puntero){
	log_info(logger,"Reservando bloque en pagina %d del pid %d",puntero->pagina,pid );
	t_heapMetadata* auxBloque= malloc(sizeof(t_heapMetadata));
	t_heapMetadata* auxBloque2= malloc(sizeof(t_heapMetadata));
	t_adminHeap * aux = malloc(sizeof(t_adminHeap));
	int offsetAux = puntero->offset;


	int i=0;
	int sizeLibreViejo;

	//void* buffer = malloc(sizeof(t_heapMetadata));

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



	auxBloque = (t_heapMetadata*)solicitarBytesAMemoria(memoria, logger, pid, puntero->pagina, offsetAux, sizeof(t_heapMetadata));




	//memcpy(&auxBloque, buffer, sizeof(t_heapMetadata));


	sizeLibreViejo = auxBloque->size;
	auxBloque->uso = 1;
	auxBloque->size = size;

	//memcpy(buffer, &auxBloque, sizeof(t_heapMetadata));



	int respuesta1 = almacenarEnMemoria(memoria, logger, pid, puntero->pagina, offsetAux, sizeof(t_heapMetadata), auxBloque);



	if(respuesta1==EXIT_FAILURE_CUSTOM){
		log_error(logger, "NO PUDE ALMACENAR DATOS EN BLOQUE");
		return -1;
	}


	auxBloque2->uso = 0;
	auxBloque2->size = sizeLibreViejo - size - sizeof(t_heapMetadata);

	//memcpy(buffer, &auxBloque, sizeof(t_heapMetadata));

	int nuevoOffset = offsetAux + sizeof(t_heapMetadata)+size;



	int respuesta2 = almacenarEnMemoria(memoria, logger, pid, puntero->pagina,nuevoOffset, sizeof(t_heapMetadata), auxBloque2 );


	if(respuesta2==EXIT_FAILURE_CUSTOM){
	log_error(logger, "NO PUDE ALMACENAR DATOS EN BLOQUE");
	return -1;
	}

	log_info(logger, "Bloque de tamanio %d reservado en Heap del pid %d", size, pid);
	//free(buffer);

	return 1;
}


t_datosHeap* verificarEspacioLibreHeap( int pid, int tamanio){
	int i = 0;
	t_datosHeap* puntero = malloc(sizeof(t_datosHeap));
	t_adminHeap* aux = malloc(sizeof(t_adminHeap));
	puntero->pagina = -1;

	pthread_mutex_lock(&mutex_listaHeap);

	while(i<list_size(listaAdminHeap)){
		aux=  list_get(listaAdminHeap, i);

		if((aux->disponible >= tamanio + sizeof(t_datosHeap)) && (aux->pid==pid)){

			int res = compactarPaginaHeap(aux->pagina, aux->pid);
			if(res == EXIT_FAILURE_CUSTOM){
				puntero->pagina = EXIT_FAILURE_CUSTOM;
				puntero->offset = EXIT_FAILURE_CUSTOM;
				return puntero;
			}
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
	t_heapMetadata* aux = malloc(sizeof(t_heapMetadata));
	void * buffer = malloc(sizeof(t_heapMetadata));

	aux->uso = 0;
	aux->size = TAMPAG - sizeof(t_heapMetadata);

	memcpy(buffer, &aux, sizeof(t_heapMetadata));

	t_pedidoDePaginasKernel* pedido = malloc(sizeof(t_pedidoDePaginasKernel));
	pedido->pid = pid;
	pedido->paginasAPedir = 1;

	enviar(memoria,ASIGNAR_PAGINAS, sizeof(t_pedidoDePaginasKernel), pedido);

	pthread_mutex_lock(&mutexServidor);
	t_paquete* respuesta = recibir(memoria);
	pthread_mutex_unlock(&mutexServidor);

	resultado = respuesta->codigo_operacion;

	if(resultado== ASIGNAR_PAGINAS_FALLO){
		log_error(logger,"NO se pudo asignar la pagina al PID");
		return -1;

	}

	int respuesta2 = almacenarEnMemoria(memoria, logger, pid, pagina, 0, sizeof(t_heapMetadata), aux);



	if(respuesta2==EXIT_SUCCESS_CUSTOM){
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

int compactarPaginaHeap( int pagina, int pid){
	int res = 1;
	log_info(logger, "Compactando la pagina %d del Heap del pid %d", pagina, pid);
	int offset = 0;
	t_heapMetadata* actual = malloc(sizeof(t_heapMetadata));
	t_heapMetadata* siguiente = malloc(sizeof(t_heapMetadata));
	t_heapMetadata* buffer = malloc(sizeof(t_heapMetadata));


	actual->size= 0;

	while((offset < TAMPAG) && ((offset + sizeof(t_heapMetadata)+ actual->size) < (TAMPAG - sizeof(t_heapMetadata)))){
		int offsetAux = 0;


		actual = (t_heapMetadata*)solicitarBytesAMemoria(memoria,logger,pid, pagina, offset, sizeof(t_heapMetadata));

		//actual->uso = buffer->uso;
		//actual->size = buffer->size;

		offsetAux = offset + sizeof(t_heapMetadata)+ actual->size;
		if(offsetAux < (TAMPAG - sizeof(t_heapMetadata))){
			siguiente = (t_heapMetadata*)solicitarBytesAMemoria(memoria, logger, pid, pagina,offsetAux, sizeof(t_heapMetadata));
		}
		else
		{
			siguiente->uso = 1;
		}

		//siguiente->uso = buffer->uso;
		//siguiente->size = buffer->size;

		if(actual->uso ==0 && siguiente->uso ==0){
			actual->size = actual->size + sizeof(t_heapMetadata)+ siguiente->size;
			//memcpy(buffer, &actual, sizeof(t_heapMetadata));

			res = almacenarEnMemoria(memoria, logger, pid, pagina,offset, sizeof(t_heapMetadata), actual);

		}
		else{
			offset += sizeof(t_heapMetadata)+ actual->size;
		}

	}
	free(buffer);

	log_info(logger,"Pagina %d del heap del pid %d compactada",pagina, pid);
	return res;

}


int paginaHeapConBloqueSuficiente(int posicionPaginaHeap, int pagina, int pid, int tamanio){

	int i=0;
	t_heapMetadata* auxBloque = malloc(sizeof(t_heapMetadata));



	while(i<TAMPAG){

		auxBloque = (t_heapMetadata*)solicitarBytesAMemoria(memoria, logger, pid, pagina, i, sizeof(t_heapMetadata));
		//memcpy(&auxBloque, buffer, sizeof(t_heapMetadata));



		if(auxBloque->size >= tamanio + sizeof(t_heapMetadata) && auxBloque->uso == 0){
			log_info(logger,"Pagina %d del heap del pid %d suficiente", pagina, pid);


			return i;
		}
		else{
			i = i+sizeof(t_heapMetadata) + auxBloque->size;
		}
	}
	log_error(logger, "Pagina %d del heap del pid %d no suficiente", pagina, pid);

	return -1;
}


void finQuantum(un_socket socketCPU, t_paquete* paqueteRec){
	t_proceso* proceso;
	t_pcb* temporal;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec, socketCPU);
	eliminarProcesoDeCola(cola_exec, proceso->pcb->pid);
	pthread_mutex_unlock(&mutex_exec);
	temporal = desserializarPCB(paqueteRec->data);
	destruirPCB(proceso->pcb);
	proceso->pcb = temporal;

	if(proceso->abortado == false){
		pthread_mutex_lock(&mutex_ready);
		queue_push(cola_ready, proceso);
		pthread_mutex_unlock(&mutex_ready);
		sem_post(&sem_ready);
		log_info(logger, "PID %d encolado en ready", proceso->pcb->pid);

	}
	else{
		log_error(logger,"Pid abortado");
		abortar(proceso);
	}

	queue_push(cola_CPU_libres, (void*)socketCPU);
	sem_post(&sem_cpu);
}


void deserializarYFinalizar(un_socket socketActivo, t_paquete* paqueteRecibido, ExitCodes code){

printf("ENTRE A DESERIALIZAR Y FINALIZAR\n");

	t_pcb* temporal;
	temporal = desserializarPCB(paqueteRecibido->data);



	pthread_mutex_lock(&mutex_exec);

	t_proceso* procesoAux = malloc(sizeof(t_proceso));
	int indice;

	int a = 0;
	while(a< list_size(cola_exec->elements)){


		procesoAux = list_get(cola_exec->elements, a);

		if(procesoAux->socketCPU == socketActivo){

			indice = a;
		}

		a++;
	}


	t_proceso* proceso = malloc(sizeof(t_proceso));

	proceso = list_remove(cola_exec->elements,indice);





	pthread_mutex_unlock(&mutex_exec);

	//eliminarProcesoDeCola(cola_exec, proceso->pcb->pid);





	destruirPCB(proceso->pcb);
	proceso->pcb = temporal;

	finalizarProceso(proceso,code);

	queue_push(cola_CPU_libres, (void*)socketActivo);
	sem_post(&sem_cpu);
}


void cpuCerrada(un_socket socketCPU, t_paquete* paqueteRec){

	t_proceso* proceso;
	t_pcb* temporal;
	pthread_mutex_lock(&mutex_exec);
	proceso = obtenerProcesoSocketCPU(cola_exec, socketCPU);
	pthread_mutex_unlock(&mutex_exec);
	eliminarProcesoDeCola(cola_exec,proceso->pcb->pid);
	sacarCPUDeListas(socketCPU);
	temporal = desserializarPCB(paqueteRec->data);
	destruirPCB(proceso->pcb);
	proceso->pcb = temporal;

	if(proceso->abortado == false){
		pthread_mutex_lock(&mutex_ready);
		queue_push(cola_ready, proceso);
		pthread_mutex_unlock(&mutex_ready);
		sem_post(&sem_ready);
		log_info(logger, "PID %d encolado en ready", proceso->pcb->pid);

	}
	else{
		log_error(logger,"Pid abortado");
		abortar(proceso);
	}

}


void sacarCPUDeListas(un_socket cpu){
	int a=0;
	int w;

	while(w=(un_socket)list_get(cola_CPU_libres->elements, a)){
		if(w==cpu){
			list_remove(cola_CPU_libres->elements, a);
			sem_wait(&sem_cpu);
			log_info(logger, "Se quito a la cpu: %d de la cola de CPU Libres", (int)cpu);
		}
		a++;
	}

}







