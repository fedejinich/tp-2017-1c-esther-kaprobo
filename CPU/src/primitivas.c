#include "primitivas.h"

t_puntero definirVariable(t_nombre_variable identificador_variable){
	printf("Definiendo variable: %s", identificador_variable);
}

void asignar(t_puntero direccion_variable,t_valor_variable valor){
	printf("Asigno el valor: %i a la variable en la posicion: %i", valor, direccion_variable);
}

void finalizar(){
	printf("Termino el programa");
}
