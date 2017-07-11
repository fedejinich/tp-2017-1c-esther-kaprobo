/*
 * auxiliaresConsola.h
 *
 *  Created on: 10/7/2017
 *      Author: utnso
 */

#ifndef AUXILIARESCONSOLA_H_
#define AUXILIARESCONSOLA_H_

#include "consola.h"


typedef struct{
	int d;
	int m;
	int y;
	int H;
	int M;
	int S;
}timeAct;

//estadisticas
typedef struct{
	timeAct fechaYHoraInicio;
	timeAct fechaYHoraFin;
	int impresiones;
	int tiempo;
}estadisticas;




void iniciarConsola();
void crearArchivologgerConsola();
void cargarConfiguracion();


void mostrarEstadisticas(estadisticas estadisticasPrograma, int pid);
timeAct fechaYHora();


#endif /* AUXILIARESCONSOLA_H_ */
