/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "consola.h"
//Comentar para entregas todos los printf

int main(int argc, char **argv) {
	ejecuta = true;
	iniciarConsola();
	crearArchivoLog();
	cargarConfiguracion();

	//Interfaz con el Usuario

	while(ejecuta){
		printf("Ingrese la opcion deseada:\n");
		printf("OPCION 1 - INICIAR PROGRAMA\n");
		printf("OPCION 2 - FINALIZAR PROGRAMA\n");
		printf("OPCION 3 - DESCONECTAR CONSOLA\n");
		printf("OPCION 4 - LIMPIAR MENSAJES\n");
		printf("Su Opcion:");
		scanf("%i",&opcion);
		//opciones para consola
		switch(opcion)
		{
		case 1 :
			//Inicia nuevo programa AnSISOP, va a enviar el path del script
			iniciarPrograma();
			break;
		case 2:
			//Finaliza la ejecucion de un programa, lleva el PID correspondiente
			finalizarPrograma();
			break;
		case 3:
			//Desconecta todos los hilos de conexion con kernel, abortando todos los programas en ejecucion
			desconectarConsola();
			break;
		case 4:
			//Elimina todos los mensajes de la pantalla
			limpiarMensajes();
			break;
		default:
			printf("Opcion invalida, vuelva a intentar\n\n");
			break;
		}


	}

	kernel = conectarConElKernel();
/*
	while (1){
		char mensaje[1000];
		scanf("%s", mensaje);
		send(kernel,mensaje,strlen(mensaje),0);

	}*/
	return 0;
}

void iniciarConsola(){
	printf("%s", "\n\n====== INICIO CONSOLA ======\n\n");

}

void crearArchivoLog(){
	log = iniciarLog(ARCHIVOLOG,"Consola");
	log_info(log, "Iniciando Consola. \n");
}
void cargarConfiguracion(){

	t_config* config = config_create(getenv("archivo_configuracion_consola"));
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	printf("IP KERNEL: %s \n", ip_kernel);
	printf("PUERTO_KERNEL: %i \n", puerto_kernel);
	printf("El archivo de configuracion fue cargado con exito\n\n");
	log_info(log, "El archivo de configuracion fue cargado correctamente.\n");
}


//Iniciara programa AnSISOP recibiendo el path del script. Creará un hilo Programa
void iniciarPrograma(){
	printf("Iniciar Programa\n\n");
	return;
}

void finalizarPrograma(){
	printf("Finalizar Programa\n\n");
	return;
}

void desconectarConsola(){
	printf("Desconectar Consola\n\n");
	return;
}

void limpiarMensajes(){
	printf("Limpiar Mensajes\n\n");
	return;
}

//funcion que conecta Consola con Kernel utilizando sockets


int conectarConElKernel(){
	printf("Inicio de conexion con Kernel\n");
	// funcion deSockets
	kernel = conectar_a(ip_kernel,puerto_kernel);

	if (kernel==0){
		printf("CONSOLA: No se pudo conectar con el Kernel\n");
		exit (EXIT_FAILURE);
	}
	printf("CONSOLA: Kernel recibio nuestro pedido de conexion\n");

	printf("CONSOLA: Iniciando Handshake\n");
	bool resultado = realizar_handshake(kernel, 11);
	if (resultado){
		printf("Handshake exitoso! Conexion establecida\n");
		return kernel;
	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		exit (EXIT_FAILURE);
	}



}
