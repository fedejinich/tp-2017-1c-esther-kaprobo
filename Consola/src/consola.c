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

int ejecuta = 1;

int main(int argc, char **argv) {

	iniciarConsola();
	crearArchivoLog();
	cargarConfiguracion();

	//Interfaz con el Usuario

	while(1){
		printf("Ingrese la opcion deseada:\n");
		printf("OPCION 1 - INICIAR PROGRAMA\n");
		printf("OPCION 2 - FINALIZAR PROGRAMA\n");
		printf("OPCION 3 - DESCONECTAR CONSOLA\n");
		printf("OPCION 4 - LIMPIAR MENSAJES\n");
		printf("Su Opcion:");
		scanf("%i",&opcion);
		//opciones para consola
		switch(opcion) {
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
	pthread_create(&threadNewProgram, NULL, hiloNuevoPrograma, NULL);
	pthread_join(threadNewProgram, NULL);
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

//Funcion que es creada con un hiloPrograma
void hiloNuevoPrograma(){
	timeAct t_ini, t_fin;
	int programaFinalizado = 1;
	printf("Ingrese el nombre del archivo\n");
	scanf("%s",&nomArchi);

	//Solicito archivo, lo abro y en caso de no poder, salgo con error
	archivo = fopen(nomArchi, "r");
	if(archivo == NULL){
		printf("No se pudo abrir el archivo\n");
		exit (EXIT_FAILURE);
	}

	else
	{
		//Creo el script, en base al archivo
		script = leerArchivo(archivo);
		fclose(archivo);
		printf("enviando a ejecutar programa AnSISOP\n");
		printf("el script es: %s\n",script);
	}
	char* scriptParaEnviar = malloc(strlen(script));
	memcpy(scriptParaEnviar, script, strlen(script));

	//Abro conexion con Kernel, realizo Handshake dentro
	kernel = conectarConElKernel();

	//envio paquete con codigo 101 y el script a ejecutar al Kernel
	enviar(kernel, 101, strlen(script),scriptParaEnviar);
	t_ini = fechaYHora();

	free(scriptParaEnviar);



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
char * leerArchivo(FILE *archivo){
	fseek(archivo, 0, SEEK_END);
	long fsize = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *script = malloc(fsize + 1);
	fread(script, fsize, 1, archivo);
	script[fsize] = '\0';
	return script;
}

timeAct fechaYHora(){
	timeAct tiempoActual;
	time_t tiempo = time(0);
	struct tm * tlocal = localtime(&tiempo);
	char fecha[128];
	strftime(fecha,128, "%d/%m/%y %H:%M:%S", tlocal);
	tiempoActual.y = tlocal->tm_year;
	tiempoActual.m = tlocal->tm_mon;
	tiempoActual.d = tlocal->tm_mday;
	tiempoActual.H = tlocal->tm_hour;
	tiempoActual.M = tlocal->tm_min;
	tiempoActual.S = tlocal->tm_sec;
	return tiempoActual;
}

