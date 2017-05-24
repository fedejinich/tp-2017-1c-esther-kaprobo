#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable identificador_variable){
	printf("Definiendo variable: %s", identificador_variable);
}

t_puntero obtenerPosicionVariable (t_nombre_variable identificador_variable){
	printf("Obtener Posicion variable: %s", identificador_variable);
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	printf("Dereferenciar");
}

void asignar(t_puntero direccion_variable,t_valor_variable valor){
	printf("Asigno el valor: %i a la variable en la posicion: %i", valor, direccion_variable);
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	printf("Definiendo variable: %s", variable);
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable,t_valor_variable valor){
	printf("asignar Valor Compartida\n");
}

void irAlLabel(t_nombre_etiqueta etiqueta){
	printf("irALabel\n");
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta){
	printf("llamarSinRetorno\n");
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	printf("llamarConRetorno\n");
}
void retornar(t_valor_variable retorno){
	printf("retornar\n");
}


void finalizar(){
	printf("Termino el programa");
}

