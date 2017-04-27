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




/**
 * @NAME: iniciarLog
 * @DESC: Crea el .log del proceso, borrando el log anterior.
 * @RETURN: un logger
 */
t_log* iniciarLog(char* nombreDelArchivoLog, char* nombreDelProceso);




#endif /* COMMONS_KAPROBO_H_ */
