#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <signal.h>

//#include <Commons_Kaprobo/Commons_Kaprobo.h>
#include "Commons_Kaprobo_Consola.h"
#include "estructuras.h"
#include "hilos/hiloCPU.h"
#include "hilos/hiloKernel.h"
#include "hilos/hiloConsola.h"
#include "funcionesAuxiliares/funcionesAuxiliares.h"
#include "tablaDePaginas.h"
#include "funcionHash/funcionHash.h"
#include "cache/cache.h"
#include "operaciones.h"
#include "funcionHash/funcionHash.h"
#include "cache/cache.h"

#define CUSTOM_ERROR -23



/*
 * FUNCIONES
 * */

//Funciones de configuracion
void grandMalloc();
void cargarConfiguracion();


//Funciones
void iniciarSeniales();
void iniciarHilos();

void testFuncionHashObtengoPosicionCandidataOk();

pthread_mutex_t tablaDePaginasMutex;
pthread_mutex_t cacheMutex;




