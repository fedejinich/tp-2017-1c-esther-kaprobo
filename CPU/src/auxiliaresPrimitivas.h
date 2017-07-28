/*
 * auxiliaresPrimitvas.h
 *
 *  Created on: 2/7/2017
 *      Author: utnso
 */

#ifndef AUXILIARESPRIMITVAS_H_
#define AUXILIARESPRIMITVAS_H_

#include "CPU.h"
#include "Commons_Kaprobo_Consola.h"

t_direccion* armarDireccionPrimeraPagina();
t_direccion* armarDireccionDeArgumento();
t_direccion* proximaDireccion(int posStack, int posUltVar);
t_direccion* armarDirecccionDeFuncion();
t_direccion* proximaDireccionArg(int posStack, int posUltVar);
t_direccion* convertirPunteroADireccion(t_puntero puntero); //NOMBRES FEOS
t_puntero convertirDireccionAPuntero(t_direccion* direccion); //NOMBRES FEOS
t_direccion* armarProximaDireccion();



#endif /* AUXILIARESPRIMITVAS_H_ */
