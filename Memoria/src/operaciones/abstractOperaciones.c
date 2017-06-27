/*
 * abstractOperaciones.c
 *
 *  Created on: 20/6/2017
 *      Author: utnso
 */

#include "abstractOperaciones.h"

void retardo() {
	log_info(logger, "sleep(%i)", retardo_memoria);
	sleep(retardo_memoria/1000);
}
