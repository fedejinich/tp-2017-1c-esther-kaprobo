#ifndef COMMONS_KAPROBO_COMMONS_KAPROBO_H_
#define COMMONS_KAPROBO_COMMONS_KAPROBO_H_

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



/**
 * @NAME: iniciarLog
 * @DESC: Crea el .log del proceso, borrando el log anterior.
 * @RETURN: un logger
 */
t_log* iniciarLog(char* nombreDelArchivoLog, char* nombreDelProceso);


typedef int un_socket;
typedef struct {
	int codigo_operacion;
	int tamanio;
	void * data;
} t_paquete;

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





#endif /* COMMONS_KAPROBO_H_ */