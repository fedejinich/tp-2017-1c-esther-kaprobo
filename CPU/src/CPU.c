/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : CPU
 ============================================================================
 */

#include "CPU.h"


int flag = 0;


AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable			= definirVariable,
		.AnSISOP_obtenerPosicionVariable	= obtenerPosicionVariable,
		.AnSISOP_dereferenciar				= dereferenciar,
		.AnSISOP_asignar					= asignar,
		.AnSISOP_obtenerValorCompartida		= obtenerValorCompartida,
		.AnSISOP_asignarValorCompartida		= asignarValorCompartida,
		.AnSISOP_irAlLabel					= irAlLabel,
		.AnSISOP_llamarSinRetorno			= llamarSinRetorno,
		.AnSISOP_llamarConRetorno			= llamarConRetorno,
		.AnSISOP_finalizar					= finalizar,
		.AnSISOP_retornar					= retornar,
};


AnSISOP_kernel primitivas_kernel = {
		.AnSISOP_wait				= wait_kernel,
		.AnSISOP_signal				= signal_kernel,
		.AnSISOP_reservar			= reservarEnHeap,
		.AnSISOP_liberar			= liberarEnHeap,
		.AnSISOP_abrir				= abrirArchivo,
		.AnSISOP_borrar				= borrarArchivo,
		.AnSISOP_cerrar				= cerrarArchivo,
		.AnSISOP_moverCursor		= moverCursor,
		.AnSISOP_escribir			= escribirArchivo,
		.AnSISOP_leer				= leerArchivo

};

int main(int argc, char **argv) {
	iniciarCPU();
	sigusr1_desactivado=1;

	//manejo de señales
	signal(SIGUSR1, sig_handler);
	signal(SIGINT, sig_handler2);

	crearArchivoLog();

	char* serializado;
	pthread_mutex_init(&mutex_pcb, NULL);

	cargarConfiguracion();
	kernel = conectarConElKernel();
	memoria = conectarConMemoria();


	paq_algoritmo = recibir(kernel);
	if(paq_algoritmo->codigo_operacion  == ENVIAR_ALGORITMO){
		algoritmo = *(int*)paq_algoritmo->data;
		log_info(logger, "Obteniendo algoritmo a utilizar. \n");
	}

	liberar_paquete(paq_algoritmo);

	t_paquete* datos_kernel = recibir(kernel);

	asignarDatosDelKernel(datos_kernel);
	liberar_paquete(datos_kernel);


	while (sigusr1_desactivado){
		log_info(logger,"sigusr1: %d", sigusr1_desactivado);
		programaBloqueado = 0;
		programaAbortado = 0;
		programaFinalizado = 0;

		int quantumAux = quantum;

		log_info(logger, "Aguardando la llegada de PCB");

		flag = 1;

		datos_kernel = recibir(kernel);
		asignarDatosDelKernel(datos_kernel);
		liberar_paquete(datos_kernel);

		flag=0;

		paquete_recibido = recibir(kernel);
		log_info(logger,"Se recibio PCB, envio a deserializar");
		pcb = desserializarPCB(paquete_recibido->data);

		log_info(logger,"Program Counter: %d", pcb->programCounter);
		log_info(logger, "PID: %d", pcb->pid);

		int pid = pcb->pid;
		var_max = (tamanio_pag*(stack_size+pcb->paginasDeCodigo))-1;

		while((quantumAux!=0) && !programaBloqueado && !programaFinalizado && !programaAbortado){

			t_solicitudBytes* pedido;
			pedido = armoPedidoInstruccionAMemoria(pcb, tamanio_pag);

			char* sentencia = leer(pedido->pid, pedido->pagina, pedido->offset, pedido->tamanio);

			if(sentencia == NULL){
				programaAbortado = 1;
				log_info(logger, "Se Aborta el programa");
			}
			else{
				log_info(logger, "Se recibio instruccion para pid $d de tamanio $d", pedido->pid, pedido->tamanio);
				char* barra_cero="\0";
				memcpy(sentencia+(pedido->tamanio-1),barra_cero,1);

				log_info(logger, "Pid N°: $d, sentencia: %s", pedido->pid, depurarSentencia(sentencia));

				analizadorLinea(depurarSentencia(sentencia),&primitivas, &primitivas_kernel);

				free(sentencia);

				pcb->programCounter++;

				if(algoritmo==1){
					quantumAux--;
					usleep(quantum_sleep*1000);
				}
			}
			if(programaBloqueado){
				log_info(logger, "El programa sale por bloqueo");
				log_info(logger, "PC: %d", pcb->programCounter);
				serializado = serializarPCB(pcb);
				if(!sigusr1_desactivado){
					log_info(logger, "CPU, Bloqueado y sigusr1 activada");
					int algo;
					enviar(kernel,PROGRAMA_BLOQUEADO_SIGUSR1, sizeof(int), algo);
					//VER ESTO DEL LADO KERNEL
				}
				enviar(kernel, PROGRAMA_BLOQUEADO_SEMAFORO, ((t_pcb*)serializado)->sizeTotal, serializado);
				free(serializado);
				destruirPCB(pcb);
			}

			if(programaAbortado){
				log_info(logger,"El pid %d se aborto", pcb->pid);
				serializado = serializarPCB(pcb);
				if(!sigusr1_desactivado){
					log_info(logger, "CPU, Bloqueado y sigusr1 activada");
					int algo;
					enviar(kernel,PROGRAMA_BLOQUEADO_SIGUSR1, sizeof(int), algo);
					//VER ESTO DEL LADO KERNEL
				}
				enviar(kernel, PROGRAMA_ABORTADO, ((t_pcb*)serializado)->sizeTotal, serializado);
				free(serializado);
				destruirPCB(pcb);
			}

			if((quantumAux==0) && !programaFinalizado && !programaBloqueado && !programaAbortado){
				log_info(logger,"Se sale por fin de QUANTUM");

				serializado = serializarPCB(pcb);
				if(!sigusr1_desactivado){
					log_info(logger, "CPU, Bloqueado y sigusr1 activada");
					int algo;
					enviar(kernel,PROGRAMA_BLOQUEADO_SIGUSR1, sizeof(int), algo);
					//VER ESTO DEL LADO KERNEL
				}
				enviar(kernel,FIN_QUANTUM, ((t_pcb*)serializado)->sizeTotal, serializado);
				free(serializado);
				destruirPCB(pcb);
			}
		}

	}
	log_info(logger,"Se cierra CPU por senial SIGUSR1");
	int basura;
	enviar(kernel,SENIAL_SIGUSR1,sizeof(int), basura);

	close(kernel);
	close(memoria);


	exit(EXIT_SUCCESS);
}

void iniciarCPU(){
	printf("%s", "\n\n====== INICIO CPU ======\n\n");

}


void crearArchivoLog(){
	logger = iniciarLog(ARCHIVOLOG,"CPU");
	log_info(logger, "Iniciando CPU. \n");
}

void prueboParser(){
	printf("Inicio prueba de parser anSISOP. \n");
	//Deberia poder leer el archivo, pero no lo lee.
	archivo = fopen("testParser", "r");
	ejecutarArchivo(archivo);
	fclose(archivo);
}

void ejecutarArchivo(FILE *archivo){
	fseek(archivo, 0, SEEK_END);
	long fsize = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char sentencia[256];
	//Obtengo linea a linea y la ejecuto con el analizador.
	while(fgets(sentencia, sizeof(sentencia), archivo)){
		analizadorLinea(depurarSentencia(sentencia), &primitivas, &primitivas_kernel);
	}
}

char* depurarSentencia(char* sentencia){
	int i = strlen(sentencia);
	while (string_ends_with(sentencia, "\n")) {
	i--;
	sentencia = string_substring_until(sentencia, i);
	}
	return sentencia;
}

void cargarConfiguracion(){
	t_config* config = config_create(getenv("archivo_configuracion_CPU"));
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
	ip_memoria = config_get_string_value(config, "IP_MEMORIA");
	log_info(logger, "Cargando configuraciones. \n");
	printf("IP KERNEL: %s \n", ip_kernel);
	printf("PUERTO_KERNEL: %i \n", puerto_kernel);
	printf("IP MEMORIA: %s \n", ip_memoria);
	printf("PUERTO_MEMORIA: %i \n", puerto_memoria);
	printf("El archivo de configuracion fue cargado con exito\n");
}

void sig_handler(int signo) {
	sigusr1_desactivado = 0;
	log_info(logger,"Se detecto señal SIGUSR1, la CPU se cerrara al finalizar\n");
	if(flag==1) exit(0);
	return;
}

void sig_handler2(int signo) {
	sigusr1_desactivado = 0;
	if(flag==1) exit(0);
	programaAbortado=1;

	log_info(logger,"Se detecto señal sig int CRT C.\n");
	return;
}


//funcion que conecta CPU con Kernel utilizando sockets
int conectarConElKernel(){
	printf("Inicio de conexion con Kernel\n");
	log_info(logger, "Conectando con Kernel. \n");
	// funcion deSockets
	kernel = conectar_a(ip_kernel,puerto_kernel);

	if (kernel==0){
		printf("CPU: No se pudo conectar con el Kernel\n");
		log_info(logger, "Conexion fallida con Kernel. \n");
		exit (EXIT_FAILURE);
	}
	printf("CPU: Kernel recibio nuestro pedido de conexion\n");

	printf("CPU: Iniciando Handshake\n");
	bool resultado = realizar_handshake(kernel,12);
	if (resultado){
		printf("Handshake exitoso! Conexion establecida\n");
		log_info(logger, "Conectado con exito al Kernel. \n");
		return kernel;
	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		log_info(logger, "Conexion fallida con Kernel. \n");
		exit (EXIT_FAILURE);
	}
}

//funcion que conecta CPU con Memoria utilizando sockets
int conectarConMemoria(){
	printf("Inicio de conexion con Memoria\n");
	log_info(logger, "Conectando con Memoria. \n");
	// funcion deSockets
	memoria = conectar_a(ip_memoria,puerto_memoria);

	if (memoria==0){
		printf("CPU: No se pudo conectar con la Memoria\n");
		log_info(logger, "Conexion fallida con Memoria. \n");
		exit (EXIT_FAILURE);
	}
	printf("CPU: Memoria recibio nuestro pedido de conexion\n");

	printf("CPU: Iniciando Handshake\n");
	bool resultado = realizar_handshake(memoria,15);
	if (resultado){
		printf("Handshake exitoso! Conexion establecida\n");
		log_info(logger, "Conectado con exito a Memoria. \n");
		return memoria;
	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		log_info(logger, "Conexion fallida con Memoria. \n");
		exit (EXIT_FAILURE);
	}
}

void asignarDatosDelKernel(t_paquete* datos_kernel){
	quantum = ((t_datos_kernel*)(datos_kernel->data))->QUANTUM;
	tamanio_pag = ((t_datos_kernel*)(datos_kernel->data))->TAMANIO_PAG;
	quantum_sleep = ((t_datos_kernel*)(datos_kernel->data))->QUANTUM_SLEEP;
	stack_size = ((t_datos_kernel*)(datos_kernel->data))->STACK_SIZE;

}

t_solicitudBytes * armoPedidoInstruccionAMemoria(t_pcb* pcb, int tamanioPag){
	t_solicitudBytes* aux;

	aux->pid = pcb->pid;
	aux->pagina = pcb->indiceDeCodigo[(pcb->programCounter)*2]/tamanioPag;
	aux->offset = pcb->indiceDeCodigo[((pcb->programCounter)*2)]%tamanioPag;
	aux->tamanio = pcb->indiceDeCodigo[((pcb->programCounter)*2)+1];
	return aux;
}

char* leer(int pid, int pagina, int offset, int tamanio){

	if((tamanio + offset)<=tamanio_pag){
		enviarSolicitudAMemoria(pid, pagina, offset, tamanio);

		t_paquete* instruccion;
		instruccion = recibir(memoria);

		if(instruccion->codigo_operacion == SOLICITAR_BYTES_FALLO) return NULL;

		char* sentencia = malloc(tamanio);
		memcpy(sentencia, instruccion->data, tamanio);
		liberar_paquete(instruccion);
		return sentencia;
	}
	else {
		char* lectura1= leer(pid,pagina,offset,(tamanio_pag-offset));
		if(lectura1 == NULL) return NULL;
		char* lectura2 = leer(pid, pagina +1,0,tamanio-(tamanio_pag-offset));
		if(lectura2==NULL) return NULL;

		char* retorno = malloc((tamanio_pag-offset)+tamanio-(tamanio_pag-offset));
		memcpy(retorno, lectura1, (tamanio_pag-offset));
		memcpy(retorno + (tamanio_pag-offset), lectura2, tamanio-(tamanio_pag-offset));
		free(lectura1);
		free(lectura2);
		return retorno;
	}

}

void enviarSolicitudAMemoria(int pid, int pagina, int offset, int tamanio){
	t_solicitudBytes* pedido;
	pedido->pid = pid;
	pedido->pagina= pagina;
	pedido->offset=offset;
	pedido->tamanio=tamanio;
	log_info(logger, "PID %d solicita direccion: %d pagina %d offset %d tamanio", pid, pagina, offset, tamanio);
	enviar(memoria,SOLICITAR_BYTES, sizeof(t_solicitudBytes), pedido);
}
