#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>

/*
 * VARIABLES
 * */

//Variables de configuracion
int puerto;
int marcos;
int marco_size;
int entradas_cache;
int cache_x_proc;
char* reemplazo_cache;
int retardo_memoria;

/*
 * FUNCIONES
 * */

//Funciones de configuracion
void iniciarConsola();
void cargarConfiguracion(char* pathconf);
