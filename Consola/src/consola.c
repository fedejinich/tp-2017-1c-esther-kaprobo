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
	iniciarConsola();
	crearArchivoLog();
	cargarConfiguracion();

	//Interfaz con el Usuario

	while(1){
		pthread_mutex_lock(&mutexEjecuta);
		mostrarMenu();
		pthread_mutex_unlock(&mutexEjecuta);
		scanf("%i",&opcion);

		//opciones para consola
		switch(opcion) {
		case 1 :
			//Inicia nuevo programa AnSISOP, va a enviar el path del script
			pthread_mutex_lock(&mutexEjecuta);
			iniciarPrograma();
			break;
		case 2:
			//Finaliza la ejecucion de un programa, lleva el PID correspondiente
			pthread_mutex_lock(&mutexEjecuta);
			finalizarPrograma();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 3:
			//Desconecta todos los hilos de conexion con kernel, abortando todos los programas en ejecucion
			pthread_mutex_lock(&mutexEjecuta);
			desconectarConsola();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case 4:
			//Elimina todos los mensajes de la pantalla
			pthread_mutex_lock(&mutexEjecuta);
			limpiarMensajes();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		default:
			printf("Opcion invalida, vuelva a intentar\n\n");
			break;
		}

	}
	return 0;
}

void iniciarConsola(){
	int i;
	printf("%s", "\n\n====== INICIO CONSOLA ======\n\n");

	//Inicio matriz de PID y Sockets en 0 todo
	for(i=0;i<= MAXPID; i++){
		matriz[i] = 0;
	}

	//Inicio Mutex
	pthread_mutex_init(&mutexConexion, NULL);
	pthread_mutex_init(&mutexEjecuta, NULL);

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
	//pthread_join(threadNewProgram, NULL);
	return;
}

void finalizarPrograma(){
	int n;
	signed int soc;
	printf("Finalizar Programa\n\n");
	printf("Ingrese el PID a finalizar: \n");
	scanf("%i",&n);
	soc = matriz[n];
	//Tendria que informarle a Kernel por algun motivo que lo finalizo?
	if(soc > 0)
		close(soc);
	else
		printf("El pid %d ya no se encuentra conectado al Kernel\n", n);
	if(pthread_cancel(matrizHilos[n]) == 0){
		printf("Hilo finalizado\n");
	}
	else{
		printf("error al finalizar Hilo\n");
	}

	return;
}

void desconectarConsola(){
	int i;
	signed int soc;
	log_info(log, "Desconectar Consola\n\n");
	for( i= 0; i< MAXPID; i++){
		soc = matriz[i];
		if (soc > 0){
			close(soc);
			printf("Se cierra pid: %d \n", i);
			matriz[i] = 0;
			if(pthread_cancel(matrizHilos[i]) == 0){
				printf("Hilo finalizado\n");

				}
			else{
				printf("error al finalizar Hilo\n");
				}
		}
	}

	return;
}

void limpiarMensajes(){

	printf("Limpiar Mensajes\n\n");
	system("clear");

	return;
}

//Funcion que es creada con un hiloPrograma
void hiloNuevoPrograma(){

	//Variables de cada hilo
	signed int kernel;
	char* info_cadena;
	t_paquete* newPid;
	t_paquete* paquete;
	int programaFinalizado = 1, pid;
	estadisticas estadisticasPrograma;
	estadisticasPrograma.impresiones = 0;


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
		printf("el script es: \n%s\n",script);
	}
	char* scriptParaEnviar = malloc(strlen(script));
	memcpy(scriptParaEnviar, script, strlen(script));

	//Abro conexion con Kernel, realizo Handshake
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


	}
	else{
		printf("Fallo en el handshake, se aborta conexion\n");
		exit (EXIT_FAILURE);
		}


	//Inicio ejecución estadistica
	estadisticasPrograma.fechaYHoraInicio = fechaYHora();

	//envio paquete con codigo 101 y el script a ejecutar al Kernel
	//bloqueo mutex, para que solo exista la conexion de un hilo enviando los datos
	pthread_mutex_lock(&mutexConexion);

	enviar(kernel, 101, strlen(script),scriptParaEnviar);
	newPid = recibir(kernel);
	pid = *(int*)newPid->data;
	//me guardo el pid y el socket en la matriz para tener referencia siempre
	matriz[pid]= kernel;
	matrizHilos[pid] = pthread_self();


	printf("Se envío a ejecutar %d PID, en el socket %d \n",pid,kernel);

	pthread_mutex_unlock(&mutexConexion);

	//libero paquete y script
	liberar_paquete(newPid);
	free(scriptParaEnviar);
	pthread_mutex_unlock(&mutexEjecuta);
	//mientras el programa se ejecute, espero instrucciones de Kernel

	while(programaFinalizado){
		paquete = recibir(kernel);

		switch(paquete->codigo_operacion){

		//Programa Finalizado
		case 102:
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			pthread_mutex_lock(&mutexEjecuta);

			printf("\nFINALIZO EL PID %d \n",pid);
			mostrarEstadisticas(estadisticasPrograma, pid);
			programaFinalizado=0;
			close(kernel);
			matriz[pid]=0;


			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			pthread_exit(PTHREAD_CANCELED);

			break;

		//Imprimir texto
		case 103:
			pthread_mutex_lock(&mutexEjecuta);

			memcpy(&info_cadena, &paquete->data, paquete->tamanio);
			printf("Imprimiendo información del pid %d\n",pid);
			printf("Cadena: %s\n", info_cadena);
			estadisticasPrograma.impresiones ++;
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			break;

		//imprimir valor
		case 104:
			pthread_mutex_lock(&mutexEjecuta);

			printf("Imprimiendo información del pid %d\n",pid);
			printf("Valor: %d\n", *(int*) paquete->data);
			estadisticasPrograma.impresiones ++;
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			break;

		//Programa sin espacio en memoria
		case 105:
			pthread_mutex_lock(&mutexEjecuta);

			printf("Programa sin espacio en memoria\n");
			/* Se muestran?
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			mostrarEstadisticas(estadisticasPrograma, pid);
			*/
			programaFinalizado=0;
			close(kernel);
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			break;

		//Programa abortado por Kernel
		case 106:
			pthread_mutex_lock(&mutexEjecuta);

			printf("Programa Abortado por Kernel");
			/* Se muestran?
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			mostrarEstadisticas(estadisticasPrograma, pid);
						*/
			programaFinalizado = 0;
			close(kernel);
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		//Error por multiprogramacion
		case 108:
			pthread_mutex_lock(&mutexEjecuta);

			printf("Grado de multiprogramacion maximo\n");
			/* Se muestran?
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			mostrarEstadisticas(estadisticasPrograma, pid);
			*/
			programaFinalizado=0;
			close(kernel);
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			break;


		case -1:
			pthread_mutex_lock(&mutexEjecuta);
			printf("CONSOLA: Kernel se desconecto\n");
			close(kernel);
			liberar_paquete(paquete);
			pthread_mutex_unlock(&mutexEjecuta);
			return;
		}
	}
}

void mostrarMenu(){

	printf("\nIngrese la opcion deseada:\n");
	printf("OPCION 1 - INICIAR PROGRAMA\n");
	printf("OPCION 2 - FINALIZAR PROGRAMA\n");
	printf("OPCION 3 - DESCONECTAR CONSOLA\n");
	printf("OPCION 4 - LIMPIAR MENSAJES\n");
	printf("Su Opcion:\n");

}


//Funcion leer archivo y armar script
char * leerArchivo(FILE *archivo){
	fseek(archivo, 0, SEEK_END);
	long fsize = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *script = malloc(fsize + 1);
	fread(script, fsize, 1, archivo);
	script[fsize] = '\0';
	return script;
}

//Devuelve fecha y hora actual en estructura timeAct
timeAct fechaYHora(){
	timeAct tiempoActual;
	time_t tiempo = time(0);
	struct tm * tlocal = localtime(&tiempo);
	char fecha[128];
	strftime(fecha,128, "%d/%m/%y %H:%M:%S", tlocal);
	tiempoActual.y = tlocal->tm_year+1900;
	tiempoActual.m = tlocal->tm_mon+1;
	tiempoActual.d = tlocal->tm_mday;
	tiempoActual.H = tlocal->tm_hour;
	tiempoActual.M = tlocal->tm_min;
	tiempoActual.S = tlocal->tm_sec;
	return tiempoActual;
}

//Funcion que muestra las estadisticas de determinado pid
void mostrarEstadisticas(estadisticas estadisticasPrograma, int pid){
	timeAct ini = estadisticasPrograma.fechaYHoraInicio;
	timeAct fin = estadisticasPrograma.fechaYHoraFin;

	printf("Estadisticas del PID: %d \n\n",pid);
	printf("Fecha y hora de inicio: %i/%i/%i - %i:%i:%i\n", ini.d,ini.m,ini.y, ini.H, ini.M, ini.S);
	printf("Fecha y hora de fin: %i/%i/%i - %i:%i:%i\n", fin.d,fin.m,fin.y, fin.H, fin.M, fin.S);
	printf("Cantidad de impresiones: %d \n", estadisticasPrograma.impresiones);
	int retardo = (fin.M - ini.M)*60 + fin.S - ini.S;
	printf("Tiempo total de ejecución: %d \n\n", retardo);
}
