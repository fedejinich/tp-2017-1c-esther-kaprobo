/*
 * auxiliaresPrimitvas.h
 *
 *  Created on: 2/7/2017
 *      Author: utnso
 */

#ifndef AUXILIARESPRIMITVAS_H_
#define AUXILIARESPRIMITVAS_H_

#include "CPU.h"

t_direccion* armarDireccionPrimeraPagina();
t_direccion* armarDireccionDeArgumento();
t_direccion* proximaDireccion(int posStack, int posUltVar);
t_direccion* armarDirecccionDeFuncion();
t_direccion* proximaDireccionArg(int posStack, int posUltVar);
t_direccion* convertirPunteroADireccion(int puntero); //NOMBRES FEOS
int convertirDireccionAPuntero(t_direccion* direccion); //NOMBRES FEOS
int armarProximaDireccion();
void destruirPCB(t_pcb* pcb);


#endif /* AUXILIARESPRIMITVAS_H_ */
