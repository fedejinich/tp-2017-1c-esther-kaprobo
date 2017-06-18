/*
 * funcionHash.h
 *
 *  Created on: 18/6/2017
 *      Author: utnso
 */

#ifndef FUNCIONHASH_H_
#define FUNCIONHASH_H_

#include "../Memoria.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>

t_list** overflow;
int CANTIDAD_DE_MARCOS;
unsigned int calcularPosicion(int pid, int num_pagina);
void inicializarOverflow(int cantidad_de_marcos);
void agregarSiguienteEnOverflow(int pos_inicial, int nro_frame);
int buscarEnOverflow(int indice, int pid, int pagina);
void borrarDeOverflow(int pos_inicial, int frame);
int esPaginaCorrecta(int pos_candidata, int pid, int pagina);

#endif /* FUNCIONHASH_H_ */
