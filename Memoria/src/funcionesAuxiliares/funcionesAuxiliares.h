/*
 * funcionesAuxiliares.h
 *
 *  Created on: 17/5/2017
 *      Author: utnso
 */

#ifndef FUNCIONESAUXILIARES_FUNCIONESAUXILIARES_H_
#define FUNCIONESAUXILIARES_FUNCIONESAUXILIARES_H_

#include "../Memoria.h"

/**
* @NAME: getCantidadDeEntradasPorFrame
* @DESC: Retorna la cantidad de entradas de tabla de pagina que puede contener un frame (EJ. frame_size = 256bytes -> 20 entradas)
*/
int getCantidadDeFramesTablaDePaginas();

/**
* @NAME: getLimiteFrameByOffset
* @DESC: En base a un offset me dice hasta que posicion de memoria
* puedo escribir en un frame (EJ frame_size = 256, offset = 12 -> limite = 252)
*/
int getLimiteFrameByOffset(int offset);

/**
* @NAME: numeroDeFrameBy
* @DESC: Retorna el numero frame en el que esta la entrada a la tabla de paginas
*/
int numeroDeFrameBy(int numeroDeEntradaTablaDePaginas);

/**
* @NAME: numeroDeEntradaEnFrameBy
* @DESC: Retorna el numero de entrada dentro de un frame (EJ: EntradaReal = 21 -> Entrada = 1 del Frame = 1)
*/
int numeroDeEntradaEnFrameBy(int numeroDeEntradaTablaDePaginas);

char* getCodigoDeOperacion(int codigoInt);

t_list* getCodigosParciales(char* codigo, int size);
#endif /* FUNCIONESAUXILIARES_FUNCIONESAUXILIARES_H_ */
