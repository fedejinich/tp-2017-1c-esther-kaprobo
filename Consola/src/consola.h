#ifndef CONSOLA_H_
#define CONSOLA_H_

#include "src/Commons_Kaprobo.h"

#define ARCHIVOLOG "Consola.log"

//Configuracion
char* ip_kernel;
int puerto_kernel;
t_log * log;

//Sockets
signed int kernel;

//Variables Consola
bool ejecuta;
int opcion;

//Funciones Consola
void iniciarConsola();
void limpiarArchivos();
void crearArchivoLog();
void cargarConfiguracion();
void iniciarPrograma();
void finalizarPrograma();
void desconectarConsola();
void limpiarMensajes();

//Funciones Sockets
int conectarConElKernel();

#endif
