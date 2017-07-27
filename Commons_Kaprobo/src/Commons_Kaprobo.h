#ifndef COMMONS_KAPROBO_H_
#define COMMONS_KAPROBO_H_

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>
#include <commons/string.h>
#include <commons/error.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "Estructuras.h"
#include <math.h>


#define EXIT_FAILURE_CUSTOM -23
#define EXIT_SUCCESS_CUSTOM -99

//Estructuras

	//Estructuras Memoria
	typedef struct __attribute__((packed)) t_entradaTablaDePaginas {
		int frame;
		int pid;
		int pagina;
	} t_entradaTablaDePaginas;

	//Estructuras Memoria - Kernel
	typedef struct __attribute__((packed))t_pedidoDePaginasKernel{
		int pid;
		int paginasAPedir;
	} t_pedidoDePaginasKernel;

	typedef struct __attribute__((packed))t_asignarPaginasKernel {
		int pid;
		int paginasAsignar;
	} t_asignarPaginasKernel;

	//Estructuras Memoria - CPU
	typedef struct __attribute__((packed))t_solicitudBytes{
		int pid;
		int pagina;
		int offset;
		int tamanio;
	} t_solicitudBytes;

	typedef struct __attribute__((packed))t_almacenarBytes{
		int pid;
		int pagina;
		int offset;
		int tamanio;
		void* buffer;
	} t_almacenarBytes;

typedef enum {
	HandshakeConsolaKernel = 11,
	HandshakeCPUKernel = 12,
	HandshakeMemoriaKernel = 13,
	HandshakeFileSystemKernel = 14,
	EnvioFinalizacionAConsola = 102,
	EnvioErrorAConsola = 105,
} Kernel;

typedef enum {
	finalizadoCorrectamente = 0,
	FaltaDeMemoria = -1,
	ArchivoInexistente = -2,
	IntentoDeLecturaSinPermisos = -3,
	IntentoDeEscrituraSinPermisos = -4,
	ExcepcionDeMemoria = -5,
	DesconexionDeConsola = -6,
	FinalizacionPorConsolaDeKernel = -7,
	IntentoDeReservaDeMemoriaErroneo = -8,
	NoSePuedenAsignarMasPaginas = -9,
	ErrorSinDefinicion = -20
} ExitCodes;

typedef struct {
	int codigo_operacion;
	int tamanio;
	void * data;
} t_paquete;


//Variables

typedef int un_socket;

//VARIABLE DE AVISO DE KERNEL A CPU




//FUNCIONES GENERALES
/**
 * @NAME: iniciarLog
 * @DESC: Crea el .log del proceso, borrando el log anterior.
 * @RETURN: un logger
 */
t_log* iniciarLog(char* nombreDelArchivoLog, char* nombreDelProceso);

//FUNCIONES DE NUMEROS

/** @NAME: getParteEntera @DESC: Consigue la parte entera de un double @RETURN: Retorna la parte entera de un double como int*/
int getParteEntera(double numeroDecimal);

/** @NAME: getParteEntera @DESC: Consigue la parte entera de un double @RETURN: Retorna la parte entera de un double como int*/
double getParteDecimal(double numeroDecimal);


//FUNCIONES DE SOCKETS

/**	@NAME: conectar_a
 * 	@DESC: Intenta conectarse.
 * 	@RETURN: Devuelve el socket o te avisa si hubo un error al conectarse.
 *
 */

un_socket conectar_a(char *IP, char* Port);

/**	@NAME: socket_escucha
 * 	@DESC: Configura un socket que solo le falta hacer listen.
 * 	@RETURN: Devuelve el socket listo para escuchar o te avisa si hubo un error al conectarse.
 *
 */

un_socket socket_escucha(char* IP, char* Port);

/**	@NAME: enviar
 * 	@DESC: Hace el envio de la data que le pasamos. No hay que hacer más nada.
 *
 */

void enviar(un_socket socket_envio, int codigo_operacion, int tamanioBuffer,
		void * data);

/**	@NAME: recibir
 * 	@DESC: devuelve un paquete que está en el socket pedido
 *
 */
t_paquete* recibir(un_socket socket_para_recibir);

/**	@NAME: aceptar_conexion
 * 	@DESC: devuelve el socket nuevo que se quería conectar
 *
 */
un_socket aceptar_conexion(un_socket socket_escuchador);

/**	@NAME: liberar_paquete
 * 	@DESC: libera el paquete y su data.
 *
 */
void liberar_paquete(t_paquete * paquete);

/**	@NAME: realizar_handshake
 *
 */
bool realizar_handshake(un_socket socket_del_servidor, int codigo);

/**	@NAME: esperar_handshake
 *
 */
bool esperar_handshake(un_socket socket_del_cliente, int codigo);

char* ip_local();

//CODIGOS DE OPERACION MEMORIA-KERNEL

typedef enum {
	//MEMORIA - KERNEL

	TAMANIO_PAGINA = 0,

	INICIALIZAR_PROCESO = 1,
	INICIALIZAR_PROCESO_OK = 2,
	INICIALIZAR_PROCESO_FALLO =3,

	FINALIZAR_PROCESO = 4,
	FINALIZAR_PROCESO_OK = 5,
	FINALIZAR_PROCESO_FALLO =6,

	ASIGNAR_PAGINAS = 7,
	ASIGNAR_PAGINAS_OK = 8,
	ASIGNAR_PAGINAS_FALLO = 9,

	LIBERAR_PAGINA = 10,
	LIBERAR_PAGINA_OK = 11,
	LIBERAR_PAGINA_FALLO =12,

	//MEMORIA - CPU

	PID_EN_EJECUCION = 13,
	SOLICITAR_BYTES = 14,
	SOLICITAR_BYTES_OK =15,
	SOLICITAR_BYTES_FALLO = 16,

	ALMACENAR_BYTES = 17,
	ALMACENAR_BYTES_OK =18,
	ALMACENAR_BYTES_FALLO = 19
} codigosMemoria;


//CODIGOS DE OPERACION KERNEL-CPU
typedef enum {

	DATOS_KERNEL = 0,
	PCB_SERIALIZADO = 1,
	ENVIAR_ALGORITMO = 2,
	PEDIR_SEMAFORO = 3,
	PEDIDO_SEMAFORO_OK = 4,
	PEDIDO_SEMAFORO_FALLO = 5,
	LIBERAR_SEMAFORO = 6,
	SOLICITAR_VARIABLE = 7,
	SOLICITAR_VARIABLE_OK = 8,
	ESCRIBIR_VARIABLE = 9,
	ESCRIBIR_VARIABLE_OK = 10,
	ESCRIBIR_ARCHIVO = 11,
	ABRIR_ARCHIVO = 12,
	OBTENER_DATOS = 13,
	GUARDAR_DATOS = 14,
	CERRAR_ARCHIVO = 15,
	SOLICITAR_HEAP = 16,
	PROGRAMA_FINALIZADO = 17,
	PROGRAMA_BLOQUEADO_SIGUSR1=18,
	PROGRAMA_BLOQUEADO_SEMAFORO = 19,
	PROGRAMA_ABORTADO_CPU = 20,
	FIN_QUANTUM = 21,
	DESCONEXION_CPU = 22,
	SOLICITAR_HEAP_OK = 23,
	SOLICITAR_HEAP_FALLO = 24,
	LIBERAR_HEAP =25,
	LIBERAR_HEAP_OK = 26,
	LIBERAR_HEAP_FALLO = 27,
	MOVER_CURSOR_ARCHIVO =28,
	MOVER_CURSOR_ARCHIVO_OK=29,
	MOVER_CURSOR_ARCHIVO_FALLO = 30,
	ESCRIBIR_ARCHIVO_OK = 31,
	ESCRIBIR_ARCHIVO_FALLO = 32,
	EXCEPCION_MEMORIA=33,
	ABORTADO_STACKOVERFLOW = 34,
	ABORTADO_CONSOLA = 35,
	ABORTADO_HEAP = 36,
	ABORTADO_CONSOLA_KERNEL = 37,
	ARCHIVO_NO_SE_PUDO_ABRIR = 38


} codigosKernelCPU;


//CODIGOS DE OPERACION CONSOLA-KERNEL
typedef enum{
	ENVIAR_SCRIPT = 100,
	ENVIAR_PID = 101,
	IMPRIMIR_CONSOLA = 102,
	FINALIZAR_PROGRAMA = 103,
	SIN_ESPACIO_MEMORIA = 104,
	ABORTADO_KERNEL = 105,
	ERROR_MULTIPROGRAMACION = 106,
	ABORTADO_CPU = 107,
	FINALIZAR_PROGRAMA_DESDE_CONSOLA = 108
} codigosKernelConsola;

//CODIGOS DE OPERACION KERNEL-FILE SYSTEM
typedef enum{
	VALIDAR_ARCHIVO = 200,
	VALIDAR_ARCHIVO_OK = 201,
	VALIDAR_ARCHIVO_FALLO = 202,
	CREAR_ARCHIVO = 203,
	SIN_ESPACIO_FS = 204,
	CREAR_ARCHIVO_OK = 205,
	BORRAR_ARCHIVO = 206,
	BORRAR_ARCHIVO_OK = 207,
	BORRAR_ARCHIVO_FALLO = 208,
	SOLICITUD_OBTENCION_DATOS = 209,
	SOLICITUD_OBTENCION_DATOS_OK = 210,
	SOLICITUD_OBTENCION_DATOS_FALLO = 211,
	SOLICITUD_GUARDADO_DATOS = 212,
	SOLICITUD_GUARDADO_DATOS_OK = 213,
	SOLICITUD_GUARDADO_DATOS_FALLO = 214,

} codigosKernelFileSystem;




int cantidadPaginasCodigo(char* codigo);

int almacenarEnMemoria(un_socket socketMemoria, t_log* logger, int pid, int pagina, int offset, int tamanioBuffer, void* buffer);
void* solicitarBytesAMemoria(un_socket socketMemoria, t_log* logger, int pid, int pagina, int offset, int tamanio);



#endif /* COMMONS_KAPROBO_H_ */
