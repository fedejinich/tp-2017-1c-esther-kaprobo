/*
 * hiloPrograma.c
 *
 *  Created on: 10/7/2017
 *      Author: utnso
 */


#include "hiloPrograma.h"


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
		log_error(logger, "NO SE PUDO ABRIR EL ARCHIVO");
		exit (EXIT_FAILURE);
	}

	else
	{
		//Creo el script, en base al archivo
		script = leerArchivo(archivo);
		fclose(archivo);
	}

	char* scriptParaEnviar = malloc(strlen(script));
	memcpy(scriptParaEnviar, script, strlen(script));

	log_debug(logger, "Se pudo armar el Script correctamente");

	//Abro conexion con Kernel, realizo Handshake
	log_info(logger,"Inicio de conexion con Kernel");

	// funcion deSockets
	//kernel = conectar_a(ip_kernel,puerto_kernel);

	kernel = conectar_a(ip_kernel, puerto_kernel);

	printf("KERNEL: %d\n", kernel);

	if (kernel < 0){
		log_warning(logger,"CONSOLA: No se pudo conectar con el Kernel");
		exit (EXIT_FAILURE);
	}
	log_info(logger,"CONSOLA: Kernel recibio nuestro pedido de conexion, iniciando HANDSHAKE");

	bool resultado = realizar_handshake(kernel, HandshakeConsolaKernel);
	if (resultado){
		log_info(logger,"Handshake exitoso! Conexion establecida");
	}
	else{
		log_error(logger,"Fallo en el handshake, se aborta conexion");
		exit (EXIT_FAILURE);
		}


	//Inicio ejecución estadistica
	estadisticasPrograma.fechaYHoraInicio = fechaYHora();

	//envio paquete con codigo 101 y el script a ejecutar al Kernel
	//bloqueo mutex, para que solo exista la conexion de un hilo enviando los datos
	pthread_mutex_lock(&mutexConexion);

	enviar(kernel, ENVIAR_SCRIPT, strlen(script)+1,scriptParaEnviar);

	log_info(logger,"Se envio el script a ejecutar");
	newPid = recibir(kernel);
	if(newPid->codigo_operacion == ENVIAR_PID){
		pid = *(int*)newPid->data;
		log_info(logger, "Se recibió el PID %d", pid);

		//me guardo el pid y el socket en la matriz para tener referencia siempre
		matriz[pid]= kernel;
		matrizHilos[pid] = pthread_self();
	}
	else
		log_error(logger,"fallo envio pid");

	pthread_mutex_unlock(&mutexConexion);

	//libero paquete y script
	liberar_paquete(newPid);

	free(scriptParaEnviar);
	free(script);
	pthread_mutex_unlock(&mutexEjecuta);
	//mientras el programa se ejecute, espero instrucciones de Kernel

	while(programaFinalizado){
		paquete = recibir(kernel);

		switch(paquete->codigo_operacion){

		//Programa Finalizado
		case FINALIZAR_PROGRAMA:
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			pthread_mutex_lock(&mutexEjecuta);

			printf("\nFINALIZO EL PID %d \n",*(int*)paquete->data);
			mostrarEstadisticas(estadisticasPrograma, pid);
			programaFinalizado=0;

			close(kernel);
			matriz[pid]=0;
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			pthread_exit(PTHREAD_CANCELED);

			break;

		//Imprimir
		case IMPRIMIR_CONSOLA:
			pthread_mutex_lock(&mutexEjecuta);

			log_info(logger,"Imprimiendo información del pid %d",pid);
			printf("%s\n", (char*)paquete->data);
			estadisticasPrograma.impresiones ++;
			mostrarMenu();
			free(info_cadena);
			pthread_mutex_unlock(&mutexEjecuta);
			break;

		//Programa sin espacio en memoria
		case SIN_ESPACIO_MEMORIA:
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
		case ABORTADO_KERNEL:
			pthread_mutex_lock(&mutexEjecuta);

			printf("Pid %d Abortado por Kernel", pid);
			/* Se muestran?
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			mostrarEstadisticas(estadisticasPrograma, pid);
						*/
			programaFinalizado = 0;
			close(kernel);
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		//Error por multiprogramacion
		case ERROR_MULTIPROGRAMACION:
			pthread_mutex_lock(&mutexEjecuta);

			printf("Grado de multiprogramacion maximo para el pid %d\n", pid);
			/* Se muestran?
			estadisticasPrograma.fechaYHoraFin = fechaYHora();
			mostrarEstadisticas(estadisticasPrograma, pid);
			*/
			programaFinalizado=0;
			close(kernel);
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case ABORTADO_CPU:
			pthread_mutex_lock(&mutexEjecuta);
			printf("Pid %d abortado por CPU\n", pid);
			programaFinalizado=0;
			close(kernel);
			mostrarMenu();
			pthread_mutex_unlock(&mutexEjecuta);
			break;
		case -1:
			pthread_mutex_lock(&mutexEjecuta);
			printf("CONSOLA: Kernel se desconecto\n");
			close(kernel);
			matriz[pid]=0;
			liberar_paquete(paquete);
			pthread_mutex_unlock(&mutexEjecuta);
			return;
		}
	}
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
