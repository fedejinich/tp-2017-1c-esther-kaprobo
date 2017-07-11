/*
 * auxiliaresPrimitivas.c

 *
 *  Created on: 2/7/2017
 *      Author: utnso
 */

#include "auxiliaresPrimitivas.h"


t_direccion* armarDireccionPrimeraPagina() {
	t_direccion *direccion = malloc(sizeof(t_direccion));
	direccion->offset = 0;
	direccion->size = 4;
	direccion->pagina = primeraPagina();

	if(direccion->pagina == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error en armarDireccionPrimeraPagina()");
		return EXIT_FAILURE_CUSTOM;
	}

	return direccion;
}

int primeraPagina() {
	int pagina = pcb->paginasDeCodigo +1 ; //por que es asi?

	if(pagina < 0) {
		log_error(logger, "Error en primeraPagina()");
		return EXIT_FAILURE_CUSTOM;
	}

	return pagina;
}

t_direccion* armarDireccionDeArgumento() {
	t_direccion* direccion;
	if(((t_contexto*)list_get(pcb->contextoActual, pcb->sizeContextoActual-1))->sizeArgs == 0){
		log_info(logger,"No hay argumentos");
		int posicionStackAnterior = pcb->sizeContextoActual - 2;
		int posicionUltimaVariable = ((t_contexto*)(list_get(pcb->contextoActual, pcb->sizeContextoActual - 2)))->sizeVars - 1;
		direccion = proximaDireccion(posicionStackAnterior, posicionUltimaVariable);
	}

	else {
		log_info(logger,"Busco ultimo argumento\n");
		int posicionStackActual = pcb->sizeContextoActual-1;
		int posicionUltimoArgumento = ((t_contexto*)(list_get(pcb->contextoActual, pcb->sizeContextoActual-1)))->sizeArgs-1;
		direccion = proximaDireccion(posicionStackActual, posicionUltimoArgumento);
	}

	if(direccion == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error armarDireccionArgumento()");
		return EXIT_FAILURE_CUSTOM;
	}

	return direccion;
}

t_direccion* proximaDireccion(int posStack, int posUltVar) {
	t_direccion* direccion = malloc(sizeof(t_direccion));
	direccion->pagina = -1;
	direccion->offset = -1;
	direccion->size = -1;

	int offsetNuevo = ((t_variable*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posStack)))->vars, posUltVar)))->direccion->offset + 4;
	if(offsetNuevo >= tamanioPagina){
		direccion->pagina = ((t_variable*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posStack)))->vars, posUltVar)))->direccion->pagina + 1;
		direccion->offset = 0;
		direccion->size=4;
	}

	else{
		direccion->pagina = ((t_variable*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posStack)))->vars, posUltVar)))->direccion->pagina;
		direccion->offset = offsetNuevo;
		direccion->size = 4;
	}

	if(direccion->pagina == -1 || direccion->offset == -1 || direccion->size == -1) {
		log_error(logger, "Error en proximaDireccion. Posicion Stack %i, Posicion Ultima Variable %i", posStack, posUltVar);
		return EXIT_FAILURE_CUSTOM;
	}

	return direccion;
}

t_direccion* armarDirecccionDeFuncion() {
	t_direccion* direccion;
	t_contexto* contextoActual = (t_contexto*)list_get(pcb->contextoActual, pcb->sizeContextoActual-1);

	if(contextoActual->sizeArgs == 0 && contextoActual->sizeVars == 0) {
		log_info(logger,"Entrando a definir variable en contexto sin argumentos y sin vars");
		int posicionStackAnterior = pcb->sizeContextoActual-2;
		int posicionUltimaVariable = ((t_contexto*)(list_get(pcb->contextoActual, pcb->sizeContextoActual-2)))->sizeVars-1;
		direccion = proximaDireccion(posicionStackAnterior, posicionUltimaVariable);
	}
	else if(contextoActual->sizeArgs != 0 && contextoActual->sizeVars == 0) {
		log_info(logger,"Entrando a definir variable a partir del ultimo argumento");
		int posicionStackActual = pcb->sizeContextoActual-1;
		int posicionUltimoArgumento = contextoActual->sizeArgs-1;
		direccion = proximaDireccionArg(posicionStackActual, posicionUltimoArgumento);
	}
	else if(((t_contexto*)list_get(pcb->contextoActual, pcb->sizeContextoActual-1))->sizeVars != 0){
		log_info(logger,"Entrando a definir variable a partir de la ultima variable");
		int posicionStackActual = pcb->sizeContextoActual-1;
		int posicionUltimaVariable = ((t_contexto*)(list_get(pcb->contextoActual, pcb->sizeContextoActual-1)))->sizeVars-1;
		direccion = proximaDireccion(posicionStackActual, posicionUltimaVariable);
	}

	if(direccion == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error en armarDireccionDeFuncion()");
		return EXIT_FAILURE_CUSTOM;
	}

	return direccion;
}

t_direccion* armarProximaDireccion(){
	int ultimaPosicionStack = pcb->sizeContextoActual - 1;
	int posicionUltimaVariable = ((t_contexto*) (list_get(pcb->contextoActual, ultimaPosicionStack)))->sizeVars - 1;
	t_direccion* direccion = proximaDireccion(ultimaPosicionStack, posicionUltimaVariable);

	if(direccion == EXIT_FAILURE_CUSTOM) {
		log_error(logger, "Error en armarProximaDireccion()");
		return EXIT_FAILURE_CUSTOM;
	}

	return direccion;
}

t_direccion* proximaDireccionArg(int posStack, int posUltVar) {
	t_direccion* direccion = malloc(sizeof(t_direccion));
	direccion->pagina = -1;
	direccion->offset = -1;
	direccion->size = -1;

	log_info(logger,"Entre a proximadirecArg");
	int offset = ((t_direccion*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posStack)))->args, posUltVar)))->offset + 4;
	log_info(logger,"Offset siguiente es %i", offset);

	if(offset >= tamanioPagina){
		direccion->pagina = ((t_direccion*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posStack)))->args, posUltVar)))->pagina + 1;
		direccion->offset = 0;
		direccion->size=4;
	} else {
		direccion->pagina = ((t_direccion*)(list_get(((t_contexto*)(list_get(pcb->contextoActual, posStack)))->args, posUltVar)))->pagina;
		direccion->offset = offset;
		direccion->size=4;
	}

	if(direccion->pagina == -1 || direccion->offset == -1 || direccion->size == -1) {
		log_error(logger, "Error en proximaDireccion. Posicion Stack %i, Posicion Ultima Variable %i", posStack, posUltVar);
		return EXIT_FAILURE_CUSTOM;
	}

	return direccion;
}

int convertirDireccionAPuntero(t_direccion* direccion) {
	int direccion_real,pagina,offset;
	pagina = (direccion->pagina) * tamanioPagina;
	offset = direccion->offset;
	direccion_real = pagina + offset;
	return direccion_real;
}

t_direccion* convertirPunteroADireccion(int puntero) {
	t_direccion* direccion = malloc(sizeof(t_direccion));
	direccion->pagina = -1;
	direccion->offset = -1;
	direccion->size = -1;
	if(tamanio_pag > puntero){
		direccion->pagina = 0;
		direccion->offset = puntero;
		direccion->size = 4;
	} else {
		direccion->pagina = (puntero / tamanio_pag);
		direccion->offset = puntero % tamanio_pag;
		direccion->size = 4;
	}
	if(direccion->pagina == -1 || direccion->offset == -1 || direccion->size == -1) {
		log_error(logger, "Error en convertirPunteroADireccion(%i)", puntero);
		return EXIT_FAILURE_CUSTOM;
	}
	return direccion;
}


