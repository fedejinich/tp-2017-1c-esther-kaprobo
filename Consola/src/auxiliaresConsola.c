/*
 * auxiliaresConsola.c
 *
 *  Created on: 10/7/2017
 *      Author: utnso
 */


#include "auxiliaresConsola.h"


void desconexionConsola(int signo) {

	log_warning(logger,"Se detecto señal sig int CRT C.");
	desconectarConsola();
	exit(0);
	return;
}


void limpiarArchivos(){
	remove(ARCHIVOLOG);
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

void crearArchivologgerConsola(){
	logger = iniciarLog(ARCHIVOLOG,"Consola");
	log_info(logger, "Iniciando Consola. \n");
}
void cargarConfiguracion(){

	t_config* config = config_create("consola.config");
	puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	log_info(logger,"IP KERNEL: %s", ip_kernel);
	log_info(logger,"PUERTO_KERNEL: %i", puerto_kernel);
	log_info(logger, "El archivo de configuracion fue cargado correctamente.\n");
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


