#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>

/*
 * VARIABLES
 * */

//Variables de configuracion
int puerto_prog;
int puerto_cpu;
int ip_memoria;
int puerto_memoria;
int ip_fs;
int puerto_fs;
int quantum;
int quantum_sleep;
char* algoritmo;
int grado_multiprog;


/*
 * FUNCIONES
 * */

//Funciones de configuracion
void cargarConfiguracion();
void mostrarConfiguracion();
