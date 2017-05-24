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
};

int main(int argc, char **argv) {

	iniciarCPU();
	sigusr1_desactivado=1;
	//manejo de señales
	signal(SIGUSR1, sig_handler);
	signal(SIGINT, sig_handler2);

	crearArchivoLog();
	cargarConfiguracion();
	//No funciona conexiones.
	kernel = conectarConElKernel();
	memoria = conectarConMemoria();
	//prueboParser();
	while (1){
		paquete_recibido = recibir(kernel);
		pcb = deserializarPCB(paquete_recibido->data);
		int pid = pcb->pid;
		free(paquete_recibido);
	}

	return 0;
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
char * leerArchivo(FILE *archivo){
	fseek(archivo, 0, SEEK_END);
	long fsize = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *script = malloc(fsize + 1);
	fread(script, fsize, 1, archivo);
	script[fsize] = '\0';
	return script;
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
	//programaAbortado=1;

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
		//exit (EXIT_FAILURE);
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


//Funcion que toma lo que envio el Kernel y lo convierte en el PCB.
t_pcb* deserializarPCB(char* buffer){
	t_pcb* pcb;

	pcb = malloc(sizeof(t_pcb));
	memcpy(pcb, buffer, sizeof(t_pcb));
	buffer += sizeof(t_pcb);

	pcb->pid = malloc(sizeof(int));
	memcpy(pcb->pid, buffer, sizeof(int));
	buffer =+ sizeof(int);

	pcb->pageCounter = malloc(sizeof(int));
	memcpy(pcb->pageCounter, buffer, sizeof(int));
	buffer =+ sizeof(int);

	return pcb;
}
