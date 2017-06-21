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

//Estructuras

	//Estructuras Memoria - Kernel
	typedef struct {
		int pid;
		int paginasAPedir;
	} t_pedidoDePaginasKernel;

	//Estructuras Memoria - CPU
	typedef struct {
		int pid;
		int pagina;
		int offset;
		int tamanio;
	} t_solicitudBytes;

	typedef struct {
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
}Kernel;

typedef enum {
	FaltaDeMemoria = -1,
	ArchvioInexistente = -2,
	IntentoDeLecturaSinPermisos = -3,
	IntentoDeEscrituraSinPermisos = -4,
	ExcepcionDeMemoria = -5,
	DesconexionDeConsola = -6,
	FinalizacionPorConsolaDeKernel = -7,
	IntentoDeReservaDeMemoriaErroneo = -8,
	NoSePuedenAsignarMasPaginas = -9,
	ErrorSinDefinicion = -20
}ExitCodes;

typedef struct {
	int codigo_operacion;
	int tamanio;
	void * data;
} t_paquete;


//Variables

typedef int un_socket;


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

void enviar(un_socket socket_envio, int codigo_operacion, int tamanio,
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



//CODIGOS DE OPERACION MEMORIA-KERNEL

typedef enum {
	TAMANIO_PAGINA,

	INICIALIZAR_PROCESO,
	INICIALIZAR_PROCESO_OK,
	INICIALIZAR_PROCESO_FALLO,

	FINALIZAR_PROCESO,
	FINALIZAR_PROCESO_OK,
	FINALIZAR_PROCESO_FALLO,

	ASIGNAR_PAGINAS,
	ASIGNAR_PAGINAS_OK,
	ASIGNAR_PAGINAS_FALLO
} codigosMemoriaKernel;

//CODIGOS DE OPERACION MEMORIA-CPU

typedef enum {
	SOLICITAR_BYTES,
	SOLICITAR_BYTES_OK,
	SOLICITAR_BYTES_FALLO,

	ALMACENAR_BYTES,
	ALMACENAR_BYTES_OK,
	ALMACENAR_BYTES_FALLO
} codigosMemoriaCPU;

//CODIGOS DE OPERACION KERNEL-CPU
typedef enum {
	ENVIAR_ALGORITMO
}codigosKernelCPU;

//CODIGOS DE OPERACION CONSOLA-KERNEL
typedef enum{
	ENVIAR_SCRIPT = 101,
	ENVIAR_PID
};

#endif /* COMMONS_KAPROBO_H_ */
